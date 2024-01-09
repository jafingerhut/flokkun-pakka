(ns flokkun-pakka.rules
  (:require [clojure.set :as set]
            [clojure.string :as str]))


(defn concat-vec [v1 v2]
  (loop [ret v1, remaining v2]
    (if (seq remaining)
      (recur (conj ret (first remaining))
             (next remaining))
      ret)))

(comment
  (= [1 2 3] (concat-vec [1 2] [3]))
  (= [1 2 3 4 5] (concat-vec [1 2] [3 4 5]))
  (= [1 2] (concat-vec [1 2] []))
)

(defn is-range-match-criteria? [mc]
  (and (= (:kind mc) :range)
       (contains? mc :low)
       (contains? mc :high)
       (integer? (:low mc))
       (integer? (:high mc))
       (<= (:low mc) (:high mc))))

(defn range-match-criteria-is-also-optional? [mc min-value max-value]
  (and (is-range-match-criteria? mc)
       (<= min-value (:low mc))
       (<= (:high mc) max-value)
       (or (= (:low mc) (:high mc))
           (and (= min-value (:low mc))
                (= (:high mc) max-value)))))

(comment

(= false (is-range-match-criteria? {:kind :range :low 5}))
(= true (is-range-match-criteria? {:kind :range :low 5 :high 10}))
(= false (is-range-match-criteria? {:kind :range :low 5 :high "10"}))
(= false (is-range-match-criteria? {:kind :range :low 5 :high 4}))

(= false (range-match-criteria-is-also-optional? {:kind :range :low 5 :high 10} 0 255))
(= true (range-match-criteria-is-also-optional? {:kind :range :low 10 :high 10} 0 255))
(= true (range-match-criteria-is-also-optional? {:kind :range :low 0 :high 255} 0 255))
(= false (range-match-criteria-is-also-optional? {:kind :range :low 0 :high 254} 0 255))

)

;; These are the field indexes in most of the ClassBench rule files I
;; have examined.  I believe that some also support additional fields
;; like flags and "extra" on each line.
(def cb-sa-field-idx 0)
(def cb-da-field-idx 1)
(def cb-proto-field-idx 2)
(def cb-sport-field-idx 3)
(def cb-dport-field-idx 4)

(defn port-range-class [rule field-id]
  (let [{:keys [low high]} (nth (:field rule) field-id)]
    (cond
      (and (= low 0) (= high 65535)) :wc
      (and (= low 1024) (= high 65535)) :hi
      (and (= low 0) (= high 1023)) :lo
      (and (= low high)) :em
      :else :ar)))

(defn port-pair-class [rule]
  [(port-range-class rule cb-sport-field-idx)
   (port-range-class rule cb-dport-field-idx)])

;; Several parts of a ClassBench parameter file list the 25
;; port-pair-class values in this particular order.
(def classbench-parameter-file-port-pair-class-order
  [
   [:wc :wc]
   [:wc :hi]
   [:hi :wc]
   [:hi :hi]
   [:wc :lo]
   [:lo :wc]
   [:hi :lo]
   [:lo :hi]
   [:lo :lo]
   [:wc :ar]
   [:ar :wc]
   [:hi :ar]
   [:ar :hi]
   [:wc :em]
   [:em :wc]
   [:hi :em]
   [:em :hi]
   [:lo :ar]
   [:ar :lo]
   [:lo :em]
   [:em :lo]
   [:ar :ar]
   [:ar :em]
   [:em :ar]
   [:em :em]
   ])


(defn group-by-range-mc [rules field-id]
  (group-by (fn [r]
              (select-keys (nth (:field r) field-id)
                           [:kind :low :high]))
            rules))

(defn cb-prots-param-line-for-proto [dat proto num-rules]
  (let [num-rules-for-proto (->> (vals dat)
                                 (map count)
                                 (reduce +))
        start (format "%d %.8f "
                      proto
                      (/ (double num-rules-for-proto) num-rules))
        port-pair-stats (map (fn [ppc]
                               (/ (double (count (get dat ppc [])))
                                  num-rules-for-proto))
                             classbench-parameter-file-port-pair-class-order)]
    (str start (str/join " " (map #(format "%.8f" %) port-pair-stats)))))


(defn calc-classbench-protocol-data [rules]
  (let [grouped (-> rules
               (group-by-range-mc cb-proto-field-idx)
               (update-vals #(group-by port-pair-class %)))]
    (into (sorted-map)
          (map (fn [k]
                 (let [wc-proto? (and (= (:low k) 0) (= (:high k) 255))
                       new-k (if wc-proto? -1 (:low k))]
                   [new-k
                    (cb-prots-param-line-for-proto (get grouped k)
                                                   new-k (count rules))]))
               (keys grouped)))))


(defn calc-classbench-port-range-data-helper [rules field-idx]
  (let [n (count rules)
        grouped-by-mc (group-by #(nth (:field %) field-idx) rules)
        mc-counts (->> grouped-by-mc
                       (map (fn [[match-criteria rules-with-that-mc]]
                              [match-criteria (count rules-with-that-mc)]))
                       (sort-by (juxt #(:low (first %)) #(:high (first %))
                                      second)
                                #(compare %2 %1)))]
    (mapv (fn [[match-criteria cnt]]
            (format "%.8f\t%d:%d"
                    (/ (double cnt) n)
                    (:low match-criteria)
                    (:high match-criteria)))
          mc-counts)))


(defn calc-classbench-port-range-data [rules port-select]
  (let [field-idx (case port-select
                    :source-port cb-sport-field-idx
                    :dest-port cb-dport-field-idx
                    nil)
        port-grouped (group-by #(port-range-class % field-idx) rules)
        em-lines (calc-classbench-port-range-data-helper (:em port-grouped)
                                                         field-idx)
        ar-lines (calc-classbench-port-range-data-helper (:ar port-grouped)
                                                         field-idx)]
    {:em-lines em-lines
     :ar-lines ar-lines}))


(defn fields-disjoint [mc1 mc2]
  (or (< (:high mc1) (:low mc2))
      (< (:high mc2) (:low mc1))))


(defn rules-disjoint [r1 r2]
  (boolean (some identity (map fields-disjoint (:field r1) (:field r2)))))


(defn fields-subset [mc1 mc2]
  (and (>= (:low mc1) (:low mc2))
       (<= (:high mc1) (:high mc2))))


(defn rules-subset [r1 r2]
  (every? identity (map fields-subset (:field r1) (:field r2))))


(defn compare-rules [r1 r2]
  (if (rules-disjoint r1 r2)
    :rule-compare-disjoint
    (let [earlier-subset (rules-subset r1 r2)
          later-subset (rules-subset r2 r1)]
      (cond (and earlier-subset later-subset) :rule-compare-equal
            earlier-subset :rule-compare-earlier-strict-subset
            later-subset :rule-compare-later-strict-subset
            :else :rule-compare-conflict))))

;; Template for a case statement that exhausts all possible
;; compare-rules return values:
(comment

(case cmp-result
  :rule-compare-disjoint
  :rule-compare-equal
  :rule-compare-earlier-strict-subset
  :rule-compare-later-strict-subset
  :rule-compare-conflict
  )

)


(defn fields-intersection [mc1 mc2]
  (let [max-low (max (:low mc1) (:low mc2))
        min-high (min (:high mc1) (:high mc2))]
    {:kind :range :low max-low :high min-high}))

(defn rule-intersection [r1 r2]
  (if (rules-disjoint r1 r2)
    ;; Intentionally create the output rule so that for every field,
    ;; high < low.
    {:field (vec (repeat (count (:field r1)) {:low 1 :high 0}))}
    {:field (vec (map fields-intersection (:field r1) (:field r2)))}))


(defn rule-matchable? [maybe-add-rule cur-rules]
  ;;(println "dbg maybe-add-rule " (:field maybe-add-rule))
  (loop [remaining-cur-rules cur-rules]
    (if (seq remaining-cur-rules)
      (let [rule-kept (first remaining-cur-rules)
            cmp-result (compare-rules rule-kept maybe-add-rule)]
        ;;(println "dbg rule-kept  " (:field rule-kept))
        ;;(println "    cmp-result " cmp-result)
        (if (contains? #{:rule-compare-later-strict-subset
                         :rule-compare-equal}
                       cmp-result)
          ;; then maybe-add-rule is unmatchable because of rule-kept.
          ;; No need to keep looping through the rest of
          ;; remaining-cur-rules
          {:matchable false, :unmatchable-reason {:compare-result cmp-result
                                                  :against-earlier-rule rule-kept}}
          ;; else maybe-add-rule is (probably) matchable, but keep
          ;; checking the rest of remaining-cur-rules
          (recur (rest remaining-cur-rules))))
      ;; else finished checking remaining-cur-rules, and as far as we
      ;; can tell maybe-add-rule is matchable.
      {:matchable true})))


(defn remove-unmatchable
  ([rules]
   (remove-unmatchable rules {}))
  ([rules opts]
   (loop [remaining-rules-in rules
          rules-kept []
          unmatchable-rules []]
     (if (seq remaining-rules-in)
       (let [rule-in (first remaining-rules-in)
             res (rule-matchable? rule-in rules-kept)]
         (let [n (+ (count rules-kept) (count unmatchable-rules))]
           (when (and (:show-progress opts)
                      (not (zero? n))
                      (zero? (mod n 10000)))
             (println (format "remove-unmatchable %d done (%d kept), %d remaining ..."
                              n (count rules-kept)
                              (count remaining-rules-in)))))
         (if (:matchable res)
           (recur (rest remaining-rules-in) (conj rules-kept rule-in)
                  unmatchable-rules)
           (recur (rest remaining-rules-in) rules-kept
                  (conj unmatchable-rules
                        (assoc rule-in
                               :extra-data res)))))
       ;; else
       {:rules-kept rules-kept :unmatchable-rules unmatchable-rules}))))


;; Less powerful than remove-unmatchable because it does not find and
;; eliminate later rules that are strict subsets of earlier rules,
;; only exact duplicates, but it is nearly linear time rather than
;; quadratic time.
(defn remove-duplicates
  ([rules]
   (remove-duplicates rules {}))
  ([rules opts]
   (loop [remaining-rules-in rules
          rules-kept []
          rules-kept-set #{}
          duplicate-rules []]
     (if (seq remaining-rules-in)
       (let [rule-in (first remaining-rules-in)
             duplicate? (contains? rules-kept-set (:field rule-in))]
         (if duplicate?
           (recur (rest remaining-rules-in) rules-kept
                  rules-kept-set (conj duplicate-rules rule-in))
           (recur (rest remaining-rules-in)
                  (conj rules-kept rule-in)
                  (conj rules-kept-set (:field rule-in))
                  duplicate-rules)))
       ;; else
       {:rules-kept rules-kept :duplicate-rules duplicate-rules}))))


(defn resolve-rules-to-add [rule-to-add cur-rules-vec
                            k data-combine-fn]
  (loop [remaining-cur-rules cur-rules-vec
         ret-vec (transient (conj cur-rules-vec rule-to-add))]
    (if (seq remaining-cur-rules)
      (let [rule (first remaining-cur-rules)]
        (if (= (compare-rules rule rule-to-add) :rule-compare-conflict)
          (let [resolve-rule (assoc
                              (rule-intersection rule-to-add rule)
                              k (data-combine-fn (get rule-to-add k)
                                                 (get rule k)))]
            (recur (rest remaining-cur-rules) (conj! ret-vec resolve-rule)))
          ;; else
          (recur (rest remaining-cur-rules) ret-vec)))
      (persistent! ret-vec))))


;; For better efficiency, the set of rules should not contain any
;; rules that would be removed by remove-unmatchable, or equivalently,
;; the input should be the :rules-kept part of the output of a call to
;; remove-unmatchable.
(defn add-resolve-rules-helper [rules k data-combine-fn opts]
  (let [last-print-count (atom 0)]
    (loop [remaining-rules-in (rseq rules)
           rules-out-vec []]
      (if (seq remaining-rules-in)
        (let [rule-in (first remaining-rules-in)
              rules-out-vec (resolve-rules-to-add rule-in rules-out-vec
                                                  k data-combine-fn)]
          (when (:show-progress opts)
            (when (>= (count rules-out-vec) (+ @last-print-count 5000))
              (println (format " %d more rules added by add-resolve-rules-helper, %d more rules to process ..."
                               (- (count rules-out-vec) @last-print-count)
                               (count remaining-rules-in)))
              (reset! last-print-count (count rules-out-vec))))
          (recur (rest remaining-rules-in) rules-out-vec))
        ;; else
        (vec (rseq rules-out-vec))))))


(defn add-resolve-rules [rules opts]
  (let [k :extra-data
        r0 (map-indexed (fn [idx r] (assoc r k {:orig-rule (inc idx)}))
                        rules)
        r1 (remove-unmatchable r0 opts)
        _ (when (:show-progress opts)
            (println (format "%d rules after 1st remove-unmatchable done..."
                             (count (:rules-kept r1)))))
        r2 (add-resolve-rules-helper (:rules-kept r1) k
                                     (fn [d1 d2] {:resolve-rule-of [d1 d2]})
                                     opts)
        _ (when (:show-progress opts)
            (println (format "%d rules after add-resolve-rules-helper done ..."
                             (count r2))))
        r3 (remove-duplicates r2 opts)
        _ (when (:show-progress opts)
            (println (format "%d rules after remove-duplicates done ..."
                             (count (:rules-kept r3)))))]
    {:rules (:rules-kept r3)
     :num-unmatchable-before-add-resolve (count (:unmatchable-rules r1))
     :num-resolve-rules-created (- (count r2) (count (:rules-kept r1)))
     :num-duplicates-after-add-resolve (count (:duplicate-rules r3))}))


(defn resolve-rules-to-add2 [rule-to-add cur-rules-data
                             k data-combine-fn]
  (loop [remaining-cur-rules (:rules-out-vec cur-rules-data)
         ret-vec (transient (conj (:rules-out-vec cur-rules-data) rule-to-add))
         ret-set (:rules-out-set cur-rules-data)
         num-duplicates (:num-duplicates-in-vec cur-rules-data)]
    (if (seq remaining-cur-rules)
      (let [rule (first remaining-cur-rules)]
        (if (= (compare-rules rule rule-to-add) :rule-compare-conflict)
          (let [resolve-rule (assoc
                              (rule-intersection rule-to-add rule)
                              k (data-combine-fn (get rule-to-add k)
                                                 (get rule k)))
                dup? (contains? ret-set (:field resolve-rule))]
              (recur (rest remaining-cur-rules)
                     (conj! ret-vec resolve-rule)
                     (if dup? ret-set (conj ret-set (:field resolve-rule)))
                     (if dup? (inc num-duplicates) num-duplicates)))
          ;; else
          (recur (rest remaining-cur-rules) ret-vec ret-set num-duplicates)))
      (assoc cur-rules-data
             :rules-out-vec (persistent! ret-vec)
             :rules-out-set ret-set
             :num-duplicates-in-vec num-duplicates))))


;; add-resolve-rules-helper2 implements an idea to improve efficiency
;; of add-resolve-rules-helper: Keep count of the number of duplicate
;; rules that rules-out-vec contains, and when it exceeds some
;; threshold, only then remove them all, between itrations of the
;; loop, before continuing.  Because rules-out-vec is maintained in
;; reverse priority order, the _last_ equal rule should be kept, and
;; the earlier ones removed.

;; For better efficiency, the set of rules should not contain any
;; rules that would be removed by remove-unmatchable, or equivalently,
;; the input should be the :rules-kept part of the output of a call to
;; remove-unmatchable.
(defn add-resolve-rules-helper2 [rules k data-combine-fn opts]
  (let [last-print-count (atom 0)]
    (loop [remaining-rules-in (rseq rules)
           m {:rules-out-vec []
              :rules-out-set #{}
              :num-duplicates-in-vec 0
              :duplicates-removed 0}]
      (if (seq remaining-rules-in)
        (let [rule-in (first remaining-rules-in)
              next-m (resolve-rules-to-add2 rule-in m k data-combine-fn)
              _ (when (:show-progress opts)
                  (let [n (count (:rules-out-vec next-m))]
                    (when (>= n (+ @last-print-count 5000))
                      (println (format " %d more rules added by add-resolve-rules-helper2, %d more rules to process ..."
                                       (- n @last-print-count)
                                       (count remaining-rules-in)))
                      (reset! last-print-count n))))
              ]
          ;; Different values of the threshold should not affect the
          ;; correctness, but may affect the efficiency.
          (if (>= (:num-duplicates-in-vec next-m) 1000)
            (let [tmp (remove-duplicates (rseq (:rules-out-vec next-m)))
                  num-dups-removed (count (:duplicate-rules tmp))]
              (when (:show-progress opts)
                (println (format "%d duplicate rules removed during add-resolve-rules-helper2"
                                 num-dups-removed)))
              (recur (rest remaining-rules-in)
                     {:rules-out-vec (vec (rseq (:rules-kept tmp)))
                      :rules-out-set (:rules-out-set next-m)
                      :num-duplicates-in-vec 0
                      :duplicates-removed (+ (:duplicates-removed next-m)
                                             num-dups-removed)}))
            ;; else
            (recur (rest remaining-rules-in) next-m)))
        ;; else
        (vec (rseq (:rules-out-vec m)))))))


;; TODO: Another idea that might lead to a more efficient version of
;; add-resolve-rules-helper:
;;
;; Instead of delaying elimination of duplicates like
;; add-resolve-rules-helper2 does, do it immediately.
;;
;; I do not know of a good way to efficiently remove elements from a
;; persistent sequence.  In the absence of creating a good method for
;; doing so, or of using mutable doubly-linked lists, another approach
;; would be to simply update a Clojure vector to change a duplicate to
;; be removed to nil, a kind of "lazy deletion", and have future
;; iterations of the vector recognize and skip over nil elements.
;;
;; To quickly find the vector index that contains the duplicate,
;; maintain a map from (:field rule) vector values to the vector index
;; containing the only current index of a rule with those field match
;; criteria.
;;
;; When a later duplicate rule is created, and we want to remove the
;; earlier one, first update the vector to change the earlier
;; duplicate to nil, then update the map to contain the index of the
;; just-created rule.



(defn make-change-points [int-range-seq min-int max-int]
  (reduce (fn [m range]
            (let [m (update m (:low range)
                            #(conj (or % [])
                                   [:add-range range]))]
              (if (< (:high range) max-int)
                (update m (inc (:high range))
                        #(conj (or % [])
                               [:remove-range range]))
                m)))
          (sorted-map) int-range-seq))


;; int-range-seq is a sequence of ranges, each range represented as a
;; map with keys :low and :high.  For every range r, this condition is
;; true:
;;
;;     (<= min-int (:low r) (:high r) max-int)
;;
;; Unchecked assumption about arguments: Each distinct range occurs at
;; most once in int-range-seq.

(defn make-int-range-binary-search-tables [int-range-seq min-int max-int]
  (let [change-points (make-change-points int-range-seq min-int max-int)
        [field-val interval-updates] (first change-points)
        first-change-point-min-int? (= min-int field-val)]
    (loop [cur-matching-ranges (if first-change-point-min-int?
                                 (set (map second interval-updates))
                                 #{})
           remaining-change-points (if first-change-point-min-int?
                                     (rest change-points)
                                     change-points)
           binary-search-vec [0]
           matching-ranges-vec [cur-matching-ranges]]
      (if (seq remaining-change-points)
        ;; todo
        (let [[field-val interval-updates] (first remaining-change-points)
              next-matching-ranges (reduce
                                    (fn [matching-ranges [change range]]
                                      (case change
                                        :add-range (conj matching-ranges range)
                                        :remove-range
                                        (disj matching-ranges range)))
                                    cur-matching-ranges
                                    interval-updates)]
          (recur next-matching-ranges
                 (rest remaining-change-points)
                 (conj binary-search-vec field-val)
                 (conj matching-ranges-vec next-matching-ranges)))
        ;; else
        {:binary-search-vec binary-search-vec
         :matching-ranges matching-ranges-vec}))))


;; Calculate and return an integer value j in the range [min-int,
;; max-int] that maximizes the number of ranges in int-range-seq that
;; j lies in.  Also return a sequence of the ranges that j lies in.
;;
;; See make-int-range-binary-search-tables for assumed restrictions on
;; input values.

(defn most-int-ranges-containing-same-value [int-range-seq min-int max-int]
  (let [{:keys [binary-search-vec matching-ranges]}
        (make-int-range-binary-search-tables int-range-seq min-int max-int)

        max-idx (apply max-key (fn [idx]
                                 (count (nth matching-ranges idx)))
                       (range (count matching-ranges)))]
    {:field-value (nth binary-search-vec max-idx)
     :matching-match-criteria (nth matching-ranges max-idx)}))


(def ipv4-classbench-field-info
  [{:long-name "IPv4 source address"
    :short-name "SA"
    :min-value 0
    :max-value 0xffffffff}
   {:long-name "IPv4 destination address"
    :short-name "DA"
    :min-value 0
    :max-value 0xffffffff}
   {:long-name "IP protocol"
    :short-name "prot"
    :min-value 0
    :max-value 0xff}
   {:long-name "L4 source port"
    :short-name "sport"
    :min-value 0
    :max-value 0xffff}
   {:long-name "L4 destination port"
    :short-name "dport"
    :min-value 0
    :max-value 0xffff}])


(defn field-stats [rules field-idx field-info]
  (let [all-match-criteria (mapv (fn [r] (nth (:field r) field-idx))
                                 rules)
        distinct-match-criteria (set all-match-criteria)
        mir (most-int-ranges-containing-same-value distinct-match-criteria
                                                   (:min-value field-info)
                                                   (:max-value field-info))
        ]
    {:num-distinct-match-criteria (count distinct-match-criteria)
     :field-value-matching-max-distinct-match-criteria (:field-value mir)
     :max-distinct-match-criteria-matched-by-one-packet
     (:matching-match-criteria mir)
     :max-num-distinct-match-criteria-matched-by-one-packet
     (count (:matching-match-criteria mir))}))


(defn ipv4-classbench-rule-stats [rules]
  (mapv (fn [field-idx]
          (merge (nth ipv4-classbench-field-info field-idx)
                 (field-stats rules field-idx
                              (nth ipv4-classbench-field-info field-idx))))
        (range (count (:field (nth rules 0))))))


(defn write-ipv4-classbench-parameter-file [wrtr rules]
  (binding [*out* wrtr]
    (println "-scale")
    (println (count rules))
    (println "#")

    (println "-prots")
    (doseq [s (vals (calc-classbench-protocol-data rules))]
      (println s))
    (println "#")

    (println "-flags")
    ;; TODO
    (println "#")

    (println "-extra")
    ;; TODO
    (println "#")

    ;; L4 source/destination port arbitrary range and exact match
    ;; statistics.
    ;; spar, spem, dpar, dpem
    (doseq [[field ar-sec-name em-sec-name] [[:source-port "-spar" "-spem"]
                                             [:dest-port "-dpar" "-dpem"]]]
      (let [{:keys [ar-lines em-lines]} (calc-classbench-port-range-data
                                         rules field)]
        (println ar-sec-name)
        (doseq [s ar-lines] (println s))
        (println "#")
        (println em-sec-name)
        (doseq [s em-lines] (println s))
        (println "#")))

    ;; Source/destination address prefix length distributions (25
    ;; different section names, corresponding to the order of the
    ;; pairs in the sequence
    ;; classbench-parameter-file-port-pair-class-order).

    ;; Source/destination IP address prefix nesting thresholds (snest,
    ;; dnest), and skew (sskew, dskew)
    (println "-snest")
    (println "TODO")
    (println "#")
    (println "-sskew")
    (println "TODO")
    (println "#")
    (println "-dnest")
    (println "TODO")
    (println "#")
    (println "-dskew")
    (println "TODO")
    (println "#")

    ;; IP address prefix correlation (pcorr)
    (println "-pcorr")
    (println "TODO")
    (println "#")))


(comment

(use 'clojure.pprint)
(use 'clojure.repl)

(def ranges1 [{:low  0 :high 65535}
              {:low 10 :high 20}
              {:low 10 :high 50}
              {:low 20 :high 51}
              {:low 51 :high 100}])
(def min-int 0)
(def max-int 65535)

(def cp1 (make-change-points ranges1 min-int max-int))
(pprint cp1)

(def mr1 (make-int-range-binary-search-tables ranges1 min-int max-int))
(pprint mr1)

(def mir1 (most-int-ranges-containing-same-value ranges1 min-int max-int))
(pprint mir1)

(count (:matching-ranges mr1))

(apropos "max")
(doc max-key)

(defn num-matching-ranges [idx]
  (let [dat (nth (:matching-ranges mr1) idx)]
    (println (format "dbg idx %d cnt %d ranges %s" idx (count dat) dat)))
  (count (nth (:matching-ranges mr1) idx)))

(def max-idx (apply max-key (fn [idx] (count (nth (:matching-ranges mr1) idx))) (range (count (:matching-ranges mr1)))))
(def max-idx (apply max-key num-matching-ranges (range (count (:matching-ranges mr1)))))

max-idx
(nth (:binary-search-vec mr1) max-idx)
(nth (:matching-ranges mr1) max-idx)

)
