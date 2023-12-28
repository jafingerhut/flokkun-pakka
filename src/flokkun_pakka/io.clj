(ns flokkun-pakka.io
  (:require [clojure.java.io :as io]))


(def cb-line-re
  (re-pattern
   (apply str
          (map str [
                    #"^@"
                    #"\s*"
                    ;; IPv4 source address prefix
                    #"(\d+)\.(\d+)\.(\d+)\.(\d+)\s*/\s*(\d+)"
                    #"\s+"
                    ;; IPv4 dest address prefix
                    #"(\d+)\.(\d+)\.(\d+)\.(\d+)\s*/\s*(\d+)"
                    #"\s+"
                    
                    ;; min and max L4 source port separated by colon
                    #"(\d+)"
                    #"\s*:\s*"
                    #"(\d+)"
                    
                    ;; min and max L4 dest port separated by colon
                    #"\s+"
                    #"(\d+)"
                    #"\s*:\s*"
                    #"(\d+)"
                    #"\s+"
                    
                    ;; IP protocol value / mask separated by slash character (/)
                    #"0x([0-9a-fA-F]+)"
                    #"\s*/\s*"
                    #"0x([0-9a-fA-F]+)"
                    
                    ;; optional comment
                    #"\s*(.*)\s*"
                    #"$"
                    ]))))

(defn mask [n]
  (if (>= n 64)
    (bit-shift-left 1N 64)
    ;; TODO
    ))


(defn strings-to-ipv4-prefix [ip0-s ip1-s ip2-s ip3-s prefix-len-s]
  (let [ip0 (parse-long ip0-s)
        ip1 (parse-long ip1-s)
        ip2 (parse-long ip2-s)
        ip3 (parse-long ip3-s)
        prefix-len (parse-long prefix-len-s)]
    ;; The values cannot be negative because the regex does not match
    ;; strings with a - character in them.
    (cond
      (> ip0 255)
      {:error (format "IPv4 address decimal value %d outside of range [0,255]"
                      ip0)}
      (> ip1 255)
      {:error (format "IPv4 address decimal value %d outside of range [0,255]"
                      ip1)}
      (> ip2 255)
      {:error (format "IPv4 address decimal value %d outside of range [0,255]"
                      ip2)}
      (> ip3 255)
      {:error (format "IPv4 address decimal value %d outside of range [0,255]"
                      ip3)}
      (> prefix-len 32)
      {:error (format "IPv4 address prefix value %d outside of range [0,32]"
                      prefix-len)}
      :else
      (let [value (+ (bit-shift-left ip0 24)
                     (bit-shift-left ip1 16)
                     (bit-shift-left ip2  8)
                     ip3)
            mask (dec (bit-shift-left 1 (- 32 prefix-len)))]
        (cond
          (zero? (bit-and value mask))
          {:kind :range :low value :high (+ value mask)}
          :else
          {:error (format "IPv4 address prefix value/mask %s.%s.%s.%s/%s has non-0 bits after the prefix length, which is not supported."
                          ip0-s ip1-s ip2-s ip3-s prefix-len-s)})))))


(defn strings-to-l4-port [low-s high-s]
  (let [low (parse-long low-s)
        high (parse-long high-s)]
    ;; The values cannot be negative because the regex does not match
    ;; strings with a - character in them.
    (cond
      (> low high)
      {:error (format "L4 port low value %d is greater than high value %d"
                      low high)}

      (> high 65535)
      {:error (format "L4 port high value %d outside of range [0,65535]"
                      high)}

      :else
      {:kind :range :low low :high high})))


(defn strings-to-ip-proto [value-s mask-s]
  (let [value (Long/valueOf value-s 16)
        mask (Long/valueOf mask-s 16)]
    ;; The values cannot be negative because the regex does not match
    ;; strings with a - character in them.
    (cond
      (> value 255)
      {:error (format "IP protocol value 0x%02x outside of range [0,0xff]"
                      value)}

      (not (contains? #{0 255} mask))
      {:error (format "IP protocol mask 0x%02x must be one of 0 or 0xff"
                      mask)}

      (not= 0 (bit-and value (bit-xor 0xff mask)))
      {:error (format "IP protocol value 0x%02x has bit(s) equal to 1 where corresponding mask bit is 0 (mask is 0x%02x)"
                      value mask)}

      :else
      (if (zero? mask)
        {:kind :range :low 0 :high 255}
        {:kind :range :low value :high value}))))


(defn parse-ipv4-classbench-rule-line [s]
  (let [m (re-matches cb-line-re s)]
    (if m
      (let [sa (apply strings-to-ipv4-prefix (subvec m 1 6))
            da (apply strings-to-ipv4-prefix (subvec m 6 11))
            sport (apply strings-to-l4-port (subvec m 11 13))
            dport (apply strings-to-l4-port (subvec m 13 15))
            proto (apply strings-to-ip-proto (subvec m 15 17))]
        (cond
          (contains? sa :error) sa
          (contains? da :error) da
          (contains? sport :error) sport
          (contains? dport :error) dport
          (contains? proto :error) proto
          :else {:field [sa da proto sport dport]}))
      {:error "Line does not match expected regex for an IPv4 ClassBench rule"})))


(defn parse-ipv4-classbench-rules [rdr]
  (let [result (map-indexed
                (fn [idx line-str]
                  (assoc (parse-ipv4-classbench-rule-line line-str)
                         :input-line line-str
                         :line-number (inc idx)))
                (line-seq rdr))
        first-err (first (filter #(contains? % :error) result))]
    (if first-err
      first-err
      (doall result))))


(defn load-ipv4-classbench-rules-file [fname]
  (with-open [rdr (io/reader fname)]
    (parse-ipv4-classbench-rules rdr)))


(def prefix-mask-to-prefix-len
  (into {} (map (fn [prefix-len]
                  {(dec (bit-shift-left 1 (- 32 prefix-len)))
                   prefix-len})
                (range 33))))

(defn range-to-32bit-prefix [low high]
  (let [maybe-mask (bit-xor low high)]
    (when-not (contains? prefix-mask-to-prefix-len maybe-mask)
      (throw (IllegalArgumentException.
              (format "range-to-32bit-prefix: Range [%d,%d] with maybe-mask=%d is not equivalent to any 32-bit prefix match criteria"
                      low high maybe-mask))))
    {:value low :prefix-length (prefix-mask-to-prefix-len maybe-mask)}))

(comment
(= {:value 0 :prefix-length 32} (range-to-32bit-prefix 0 0))
(= {:value 0 :prefix-length 31} (range-to-32bit-prefix 0 1))
(range-to-32bit-prefix 0 2)
(= {:value 0 :prefix-length 30} (range-to-32bit-prefix 0 3))
(= {:value 0 :prefix-length 16} (range-to-32bit-prefix 0 65535))
)

;; TODO: Consider adding error checking and return of error status
(defn ipv4-address-match-critera->classbench-str [addr-mc]
  (let [p (range-to-32bit-prefix (:low addr-mc) (:high addr-mc))
        val (:value p)
        ip0 (bit-and 0xff (bit-shift-right val 24))
        ip1 (bit-and 0xff (bit-shift-right val 16))
        ip2 (bit-and 0xff (bit-shift-right val  8))
        ip3 (bit-and 0xff (bit-shift-right val  0))]
    (format "%d.%d.%d.%d/%d" ip0 ip1 ip2 ip3 (:prefix-length p))))


;; TODO: Consider adding error checking and return of error status
(defn proto-match-criteria->classbench-str [proto-mc]
  (let [low (:low proto-mc)
        high (:high proto-mc)]
    (cond
      (= low high) (format "0x%02x/0xff" low)
      (and (= 0 low) (= 255 high)) "0x00/0x00")))


;; TODO: Consider adding error checking and return of error status
(defn l4-port-match-criteria->classbench-str [l4-port-mc]
  (str (:low l4-port-mc) " : " (:high l4-port-mc)))


;; TODO: Consider adding error checking and return of error status
(defn rule-to-ipv4-classbench-line [r]
  (cond
    (not (map? r))
    {:error "rule-to-ipv4-classbench-line requires rules to be maps"}

    (not (contains? r :field))
    {:error "rule-to-ipv4-classbench-line requires rules to be maps with a key :field whose value is a vector of match criteria maps"}

    (not (sequential? (:field r)))
    {:error "rule-to-ipv4-classbench-line requires rules to be maps with a key :field whose value is a vector of match criteria maps"}

;;    (not (every? match-criteria? (:field r)))
;;    {:error "rule-to-ipv4-classbench-line requires rules to be maps with a key :field whose value is a vector of match criteria maps"}

    (not (= 5 (count (:field r))))
    {:error "rule-to-ipv4-classbench-line requires rules to be maps with a key :field whose value is a vector of 5 match criteria maps"}

    :else
    (let [sa (nth (:field r) 0)
          da (nth (:field r) 1)
          proto (nth (:field r) 2)
          sport (nth (:field r) 3)
          dport (nth (:field r) 4)

          sa-str (ipv4-address-match-critera->classbench-str sa)
          da-str (ipv4-address-match-critera->classbench-str da)
          proto-str (proto-match-criteria->classbench-str proto)
          sport-str (l4-port-match-criteria->classbench-str sport)
          dport-str (l4-port-match-criteria->classbench-str dport)
          extra-data-str (if (contains? r :extra-data)
                           (str "  " (pr-str (:extra-data r)))
                           "")]
      (str "@" sa-str " " da-str " " sport-str " " dport-str " " proto-str
           extra-data-str))))


(defn write-ipv4-classbench-rules [wrtr rules]
  (binding [*out* wrtr]
    (doseq [r rules]
      (let [s (rule-to-ipv4-classbench-line r)]
        (println s)))))


(defn dump-ipv4-classbench-rules-file [fname rules]
  (with-open [wrtr (io/writer fname)]
    (write-ipv4-classbench-rules wrtr rules)))



(comment

(def l1 "@64.91.106.0/23 240.0.0.0/4 0 : 65535 0 : 65535 0x06/0xFF")
(def l2 "@64.91.106.0/32 240.0.0.0/4 0 : 65535 0 : 65535 0x06/0xff  comment here")
cb-line-re

(def x (re-matches cb-line-re l1))
(def x (re-matches cb-line-re l2))

(parse_classbench-rule-line l1)
(parse_classbench-rule-line l2)

(require '[clojure.java.io :as io])

(def d1 "/Users/andy/Documents/p4-docs/flokkun-pakka/orig/song-filterset/")
(def d2 "/Users/andy/Documents/p4-docs/flokkun-pakka/flokkun-pakka/tmp/")

(def r1 (load-classbench-rules-file (str d1 "acl1_100")))
(dump-ipv4-classbench-rules-file (str d2 "acl1_100") r1)

(count r1)
(pprint (take 5 r1))
(pprint (take-last 5 r1))

)
