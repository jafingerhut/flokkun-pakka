(ns flokkun-pakka.main
  (:require [clojure.java.io :as io]
            [flokkun-pakka.rules :as r]
            [flokkun-pakka.io :as fio]))


(defn test-rule-io [m]
  (println "(clojure-version)=" (clojure-version))
  (println (format "dbg flokkun-pakka.main/test-rule-io m=%s" m))
  (when-not (and (string? (:in m))
                 (string? (:out m)))
    (println "Input map must have strings containing file names as values of keys :in and :out")
    (System/exit 1))
  (let [rules (fio/load-ipv4-classbench-rules-file (:in m))]
    (println (format "Read %d rules" (count rules)))
    (fio/dump-ipv4-classbench-rules-file (:out m) rules)))


(defn test-remove-unmatchable [m]
  (when-not (and (string? (:in m))
                 (string? (:out m))
                 (string? (:out-unmatchable m)))
    (println "Input map must have strings containing file names as values of keys :in :out :out-unmatchable")
    (System/exit 1))
  (let [rules (fio/load-ipv4-classbench-rules-file (:in m))
        _ (println (format "Read %d rules" (count rules)))
        {:keys [rules-kept unmatchable-rules]} (r/remove-unmatchable rules)]
    (println (format "Removing %d unmatchable rules, leaving %d"
                     (count unmatchable-rules)
                     (count rules-kept)))
    (fio/dump-ipv4-classbench-rules-file (:out m) rules-kept)
    (fio/dump-ipv4-classbench-rules-file (:out-unmatchable m) unmatchable-rules)))


(defn test-remove-duplicates [m]
  (when-not (and (string? (:in m))
                 (string? (:out m))
                 (string? (:out-unmatchable m)))
    (println "Input map must have strings containing file names as values of keys :in :out :out-unmatchable")
    (System/exit 1))
  (let [rules (fio/load-ipv4-classbench-rules-file (:in m))
        _ (println (format "Read %d rules" (count rules)))
        {:keys [rules-kept duplicate-rules]} (r/remove-duplicates rules)]
    (println (format "Removing %d duplicate rules, leaving %d"
                     (count duplicate-rules)
                     (count rules-kept)))
    (fio/dump-ipv4-classbench-rules-file (:out m) rules-kept)
    (fio/dump-ipv4-classbench-rules-file (:out-unmatchable m) duplicate-rules)))


(defn test-write-overlap-graph [m]
  (when-not (and (string? (:in m))
                 (string? (:out m)))
    (println "Input map must have strings containing file names as values of keys :in and :out")
    (System/exit 1))
  (let [rules (fio/load-ipv4-classbench-rules-file (:in m))]
    (println (format "Read %d rules" (count rules)))
    (with-open [wrtr (io/writer (:out m))]
      (fio/write-graphviz-overlap-graph wrtr rules))))


(defn test-add-resolve-rules [m]
  (when-not (and (string? (:in m))
                 (string? (:out m))
                 (string? (:unmatched-out m)))
    (println "Input map must have strings containing file names as values of keys :in and :out")
    (System/exit 1))
  (let [rules (fio/load-ipv4-classbench-rules-file (:in m))
        _ (println (format "%10d rules read" (count rules)))
        res (r/add-resolve-rules rules {:show-progress true})]
    (fio/dump-ipv4-classbench-rules-file (:out m) (:rules res))
    (println (format "%10d rules removed as unmatchable before add-resolve-rules"
                     (:num-unmatchable-before-add-resolve res)))
    (println (format "%10d resolve rules created during add-resolve-rules"
                     (:num-resolve-rules-created res)))
    (println (format "%10d duplicate rules removed after add-resolve-rules"
                     (:num-duplicates-after-add-resolve res)))
    (println (format "%10d rules removed as unmatchable after add-resolve-rules"
                     (:num-unmatchable-after-add-resolve res)))
    (println (format "%10d rules written" (count (:rules res))))))


(comment

(in-ns 'user)
(require '[flokkun-pakka.main :as m] :reload)
(require '[flokkun-pakka.io :as fio])
(require '[flokkun-pakka.rules :as r] :reload)

(def d1 "/Users/andy/Documents/p4-docs/flokkun-pakka/orig/song-filterset/")
(def d2 "/Users/andy/Documents/p4-docs/flokkun-pakka/tmp/")

(def r1 (fio/load-ipv4-classbench-rules-file (str d1 "acl1_100")))
(count r1)
(pprint (take 5 r1))
(def r2 (r/remove-unmatchable r1))
(count r2)
(map (fn [[k v]] [k (count v)]) r2)

(require '[clojure.java.io :as io])
(with-open [wrtr (io/writer (str d2 "acl1_100_conf.gv"))]
  (fio/write-graphviz-overlap-graph wrtr r1))

(def x1 {:field [{:kind :range, :low 1079732992, :high 1079732992} {:kind :range, :low 2327357940, :high 2327357940} {:kind :range, :low 6, :high 6} {:kind :range, :low 0, :high 65535} {:kind :range, :low 4646, :high 4646}]})

(def x2 {:field [{:kind :range, :low 1079733013, :high 1079733013} {:kind :range, :low 2162066001, :high 2162066001} {:kind :range, :low 6, :high 6} {:kind :range, :low 0, :high 65535} {:kind :range, :low 1221, :high 1221}]})

(r/rules-disjoint x1 x2)
(map r/fields-disjoint (:field x1) (:field x2))

(def e1 [1 2])
(def e2 [3 4 5])
(def e3 [6 7])

(def m1 {e1 e1, e2 e2, e3 e3})

(find m1 e1)
(identical? (key (find m1 e1)) e1)
(identical? (val (find m1 e1)) e1)
(identical? (key (find m1 [1 2])) e1)
(identical? (val (find m1 [1 2])) e1)
(identical? [1 2] e1)

)
