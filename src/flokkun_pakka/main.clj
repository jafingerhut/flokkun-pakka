(ns flokkun-pakka.main
  (:require [flokkun-pakka.io :as fio]))


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
