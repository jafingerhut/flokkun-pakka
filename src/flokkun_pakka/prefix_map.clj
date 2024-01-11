(ns flokkun-pakka.prefix-map
  (:require [clojure.pprint :as pp]
            [flokkun-pakka.bigintmath :as bi]))


;; A prefix map is like a Clojure map, in that it contains a set
;; of (key,value) pairs.  Unlike a general Clojure map, the only keys
;; supported are prefixes, i.e. a sequence of up to K bits, perhaps
;; empty.

;; In addition to support the following list of operations that normal
;; Clojure maps support:

;; + get
;; + find
;; + contains?
;; + keys
;; + vals
;; + assoc (implemented, small amount of testing done)
;; + dissoc (implemented, small amount of testing done)
;; + update

;; a binary prefix tree also supports these operations:
;;
;; + longest-prefix-match - given a search key, return the longest
;;   prefix that matches the search key that is in the tree.
;; + all-matching-prefixes - given a search key, return all prefixes
;;   that match the search key.


;; This implementation is _not_ trying to maximize performance, but to
;; be simple and effective.

;; Representation of a tree node is as a Clojure map with these keys:

;; :is-prefix? - boolean value that is true if this node represents a
;; prefix that is present in the data structure, or false if no such
;; prefix is present in the data structure.

;; :value - the value associated with this node.  This key will only
;; be present for nodes where :is-prefix? is true.

;; :left, :right - value is left/right child node (another similar
;; Clojure map).  If the :left key is omitted, there is no left child,
;; and similarly if key :right is omitted.


(def empty-prefix-map {:is-prefix? false})


(def long-bitpos-mask
  (let [x (long-array 64)]
    (doseq [j (range 64)]
      (aset x j (bit-shift-left 1 j)))
    x))

(def bigint-bitpos-mask
  (let [x (object-array 256)]
    (doseq [j (range 256)]
      (aset x j (bi/bit-shift-left' 1 j)))
    x))


(defn prefix-args-to-bit-seq [mpl value prefix-len]
;;  (println (format "dbg: prefix-args-to-bit-seq mpl %s value %s prefix-len %s"
;;                   mpl value prefix-len))
  (cond
    (<= mpl 64)
    (map (fn [j] (bit-and 1 (bit-shift-right value (- (dec mpl) j))))
         (range prefix-len))

    (<= mpl 256)
    (map (fn [j] (bi/bit-and' 1 (bi/bit-shift-right' value (- (dec mpl) j))))
         (range prefix-len))

    :else
    (throw (IllegalArgumentException.
            (format "max-prefix-len %d > 256 is not supported" mpl)))))


(comment

(prefix-args-to-bit-seq 32 0x80000000 0)
(prefix-args-to-bit-seq 32 0x80000000 1)
(prefix-args-to-bit-seq 32 0x80000000 10)
(prefix-args-to-bit-seq 32 0x87000000 10)

)


(defn pm-new-path [bit-seq val]
  (if-let [b (first bit-seq)]
    (let [k (if (zero? b) :left :right)]
      {:is-prefix? false, k (pm-new-path (rest bit-seq) val)})
    {:is-prefix? true, :value val}))


(defn pm-assoc-helper [map bit-seq val]
  (if-let [b (first bit-seq)]
    (let [k (if (zero? b) :left :right)]
      (if (contains? map k)
        (assoc map k (pm-assoc-helper (k map) (rest bit-seq) val))
        (assoc map k (pm-new-path (rest bit-seq) val))))
    ;; else
    (assoc map :is-prefix? true :value val)))


(defn prefix-map [max-prefix-len]
  (assoc empty-prefix-map :max-prefix-len max-prefix-len))


(defn pm-assoc
  ([map key val]
   (let [mpl (:max-prefix-len map)
         {:keys [value prefix-len]} key
         bit-seq (prefix-args-to-bit-seq mpl value prefix-len)]
     (pm-assoc-helper map bit-seq val)))
  ([map key val & kvs]
   (let [ret (pm-assoc map key val)]
     (if kvs
       (if (next kvs)
         (recur ret (first kvs) (second kvs) (nnext kvs))
         (throw (IllegalArgumentException.
                 "assoc expects even number of arguments after map, found odd number")))
       ret))))


(defn pm-dissoc-helper [map bit-seq]
  (if-let [b (first bit-seq)]
    (let [k (if (zero? b) :left :right)]
      (if (contains? map k)
        (let [tmp (pm-dissoc-helper (k map) (rest bit-seq))]
          (if (nil? tmp)
            (if (:is-prefix? map)
              (dissoc map k)
              (if (contains? map :max-prefix-len)
                (dissoc map k)
                nil))
            (assoc map k tmp)))
        ;; else there cannot be such a prefix in the map, so no change
        map))
    ;; else
    (if (or (contains? map :left)
            (contains? map :right))
      ;; If it has at least one child, then return the node as no
      ;; longer having a prefix, and with no value.  Its children
      ;; remain.
      (-> (dissoc map :value)
          (assoc :is-prefix? false))
      ;; If no children, then unless it is the root node, it is
      ;; removed.
      (if (contains? map :max-prefix-len)
        (-> (dissoc map :value)
            (assoc :is-prefix? false))
        ;; Remove the non-root node
        nil))))


(defn pm-dissoc
  "dissoc[iate]. Returns a new prefix-map,
  that does not contain a mapping for key(s)."
  ([map] map)
  ([map key]
   (let [mpl (:max-prefix-len map)
         {:keys [value prefix-len]} key
         bit-seq (prefix-args-to-bit-seq mpl value prefix-len)]
     (pm-dissoc-helper map bit-seq)))
  ([map key & ks]
   (let [ret (pm-dissoc map key)]
     (if ks
       (recur ret (first ks) (next ks))
       ret))))


;; Traverse every node in the prefix tree, including ones that are
;; only "intermediate" nodes, i.e. for which :is-prefix? is false.

;; In postorder, invoke the given function f with the following
;; parameters:

;; + the depth of the node, equal to the length of prefixes installed at
;;   this depth of the tree.
;; + is-prefix?
;; + has-left-child?
;; + the updated value from the left child, if there is one, otherwise nil
;; + has-right-child?
;; + the updated value from the right child, if there is one, otherwise nil
;; + the original value associated with the node, if there is one,
;;   otherwise nil

;; f should return a new value to associate with the node.

;; pm-postorder-walk returns the prefix map with the same prefixes in
;; it, but with all values updated as returned by f.
;; Note that the returned prefix-map will have a :value key, whose
;; value is the value returned by f, for all nodes, including nodes
;; with :is-prefix? equal to false.

(defn pm-postorder-walk-helper [pm depth f]
  (let [pm (if (contains? pm :left)
             (assoc pm :left
                    (pm-postorder-walk-helper (:left pm) (inc depth) f))
             pm)
        pm (if (contains? pm :right)
             (assoc pm :right
                    (pm-postorder-walk-helper (:right pm) (inc depth) f))
             pm)
;;        _ (do
;;            (println "----------------------------------------")
;;            (println (format "dbg (f depth %d is-prefix? %s has-left? %s has-right? %s"
;;                             depth (:is-prefix? pm)
;;                             (contains? pm :left)
;;                             (contains? pm :right)))
;;            (println "    (:value (:left pm)) ")
;;            (pp/pprint (:value (:left pm)))
;;            (println "    (:value (:right pm)) ")
;;            (pp/pprint (:value (:right pm)))
;;            (println "    (:value pm)")
;;            (pp/pprint (:value pm)))
        new-value (f depth (:is-prefix? pm)
                     (contains? pm :left)
                     (:value (:left pm))
                     (contains? pm :right)
                     (:value (:right pm))
                     (:value pm))
;;        _ (do
;;            (println "    new-value:")
;;            (pp/pprint new-value))
        ]
    (assoc pm :value new-value)))


(defn pm-postorder-walk [pm f]
  (let [updated-pm (pm-postorder-walk-helper pm 0 f)]
    {:updated-prefix-map updated-pm
     :root-node-value (:value updated-pm)}))


(comment

(require '[flokkun-pakka.prefix-map :as pm] :reload)
(in-ns 'user)
  
(def pm1 (pm/prefix-map 32))
(def pm2 (pm/pm-assoc pm1 {:value 0x80000000 :prefix-len 4} 5))
(def pm3 (pm/pm-assoc pm2 {:value 0xa0000000 :prefix-len 4} 10))
(def pm4 (pm/pm-assoc pm3 {:value 0xa0000000 :prefix-len 2} 15))

(def e1 *e)
e1

(pprint pm1)
(pprint pm2)
(pprint pm3)
(pprint pm4)

(def pm5 (pm/pm-dissoc pm4 {:value 0x80000000 :prefix-len 4}))

(pprint pm5)

)
