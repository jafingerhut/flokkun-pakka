(ns flokkun-pakka.rules
  (:require [clojure.set :as set]))


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


(defn resolve-rules-to-add [maybe-add-rule cur-rules-seq cur-rules-set
                            k data-combine-fn]
  ;;(println "dbg maybe-add-rule " (:field maybe-add-rule))
  (loop [remaining-cur-rules cur-rules-seq
         resolve-rules-vec []
         resolve-rules-set #{}
         num-not-added-because-equal 0]
    (if (seq remaining-cur-rules)
      (let [rule (first remaining-cur-rules)
            cmp-result (compare-rules rule maybe-add-rule)]
        ;;(println "dbg rule  " (:field rule))
        ;;(println "    cmp-result " cmp-result)
        (case cmp-result
          (:rule-compare-disjoint :rule-compare-earlier-strict-subset)
          (recur (rest remaining-cur-rules) resolve-rules-vec resolve-rules-set
                 num-not-added-because-equal)

          (:rule-compare-equal :rule-compare-later-strict-subset)
          {:matchable false}

          :rule-compare-conflict
          (let [resolve-rule (assoc
                              (rule-intersection rule maybe-add-rule)
                              k (data-combine-fn (get rule k)
                                                 (get maybe-add-rule k)))

                resolve-rule-equal-to-something-before?
                (or (contains? cur-rules-set (:field resolve-rule))
                    (contains? resolve-rules-set (:field resolve-rule)))]
            (if resolve-rule-equal-to-something-before?
              ;; then do not add it to resolve-rules-*
              (recur (rest remaining-cur-rules) resolve-rules-vec
                     resolve-rules-set
                     (inc num-not-added-because-equal))
              ;; else do add it to resolve-rules-*
              (recur (rest remaining-cur-rules)
                     (conj resolve-rules-vec resolve-rule)
                     (conj resolve-rules-set (:field resolve-rule))
                     num-not-added-because-equal)))))
      ;; else finished checking remaining-cur-rules, and as far as we
      ;; can tell maybe-add-rule is matchable.
      {:matchable true
       :resolve-rules-vec resolve-rules-vec
       :resolve-rules-set resolve-rules-set
       :num-not-added-because-equal num-not-added-because-equal})))


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
  
;; add-resolve-rules prerequisite: The set of rules should not contain
;; any rules that would be removed by remove-unmatchable, or
;; equivalently, the input should be the :rules-kept part of the
;; output of a call to remove-unmatchable.

(defn add-resolve-rules-helper [rules k data-combine-fn opts]
  (let [count-since-print (atom 0)]
    (loop [remaining-rules-in rules
           rules-out-vec []
           rules-out-set #{}
           num-unmatchable 0
           num-not-added-because-equal 0]
      (if (seq remaining-rules-in)
        (let [rule-in (first remaining-rules-in)
              res (resolve-rules-to-add rule-in rules-out-vec rules-out-set
                                        k data-combine-fn)]
          (if (:matchable res)
            (do
              (when (:show-progress opts)
                (swap! count-since-print
                       #(+ % (inc (count (:resolve-rules-vec res)))))
                (when (>= @count-since-print 5000)
                  (println (format " %d more rules added by add-resolve-rules-helper, %d more rules to process ..."
                                   @count-since-print
                                   (count remaining-rules-in)))
                  (reset! count-since-print 0)))
              (recur (rest remaining-rules-in)
                     (conj (concat-vec rules-out-vec (:resolve-rules-vec res))
                           rule-in)
                     (set/union rules-out-set (:resolve-rules-set res)
                                #{(:field rule-in)})
                     num-unmatchable
                     (+ num-not-added-because-equal
                        (:num-not-added-because-equal res))))
            (recur (rest remaining-rules-in) rules-out-vec rules-out-set
                   (inc num-unmatchable)
                   num-not-added-because-equal)))
        ;; else
        {:rules rules-out-vec
         :num-unmatchable-during-add-resolve num-unmatchable
         :num-not-added-because-equal num-not-added-because-equal}))))


(defn add-resolve-rules [rules opts]
  (let [k :rr-data
        r0 (map-indexed (fn [idx r] (assoc r k {:orig-rule (inc idx)}))
                        rules)
        r1 (remove-unmatchable r0 opts)
        _ (when (:show-progress opts)
            (println "1st remove-unmatchable done ..."))
        r2 (add-resolve-rules-helper (:rules-kept r1) k
                                     (fn [d1 d2] {:resolve-rule-of [d1 d2]})
                                     opts)
        _ (when (:show-progress opts)
            (println "add-resolve-rules-helper done ..."))
        r3 (remove-unmatchable (:rules r2) opts)]
    (assoc r2
           :rules (:rules-kept r3)
           :num-unmatchable-before-add-resolve (count (:unmatchable-rules r1))
           :num-unmatchable-after-add-resolve (count (:unmatchable-rules r3))
           :unmatchable-rules-removed-after-add-resolve (:unmatchable-rules r3))))
