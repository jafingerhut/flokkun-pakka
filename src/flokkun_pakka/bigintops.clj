(ns flokkun-pakka.bigintmath)

#_(defn bit-shift-left' [x n]
  (if (instance? clojure.lang.BigInt x)
    (if-let [b (.bipart x)]
      (clojure.lang.BigInt/fromBigInteger (.shiftLeft b n))
      (clojure.lang.BigInt/fromBigInteger (.shiftLeft (java.math.BigInteger/valueOf (.lpart x)) n)))
    ;; else
    (clojure.lang.BigInt/fromBigInteger (.shiftLeft (java.math.BigInteger/valueOf (long x)) n))))


(defn to-biginteger [x]
  (cond
    (instance? java.math.BigInteger x)
    x

    (instance? clojure.lang.BigInt x)
    (if-let [b (.bipart x)]
      b
      (java.math.BigInteger/valueOf (.lpart x)))

    :else
    (java.math.BigInteger/valueOf x)))


(defn bit-shift-left' [x n]
  (clojure.lang.BigInt/fromBigInteger (.shiftLeft (to-biginteger x) n)))


#_(defn bit-shift-right' [x n]
  (if (instance? clojure.lang.BigInt x)
    (if-let [b (.bipart x)]
      (clojure.lang.BigInt/fromBigInteger (.shiftRight b n))
      (clojure.lang.BigInt/fromBigInteger (.shiftRight (java.math.BigInteger/valueOf (.lpart x)) n)))
    ;; else
    (clojure.lang.BigInt/fromBigInteger (.shiftRight (java.math.BigInteger/valueOf (long x)) n))))


(defn bit-shift-right' [x n]
  (clojure.lang.BigInt/fromBigInteger (.shiftRight (to-biginteger x) n)))


#_(defn bit-not' [x]
  (if (instance? clojure.lang.BigInt x)
    (if-let [b (.bipart x)]
      (clojure.lang.BigInt/fromBigInteger (.not b))
      (clojure.lang.BigInt/fromBigInteger (.not (java.math.BigInteger/valueOf (.lpart x)))))
    ;; else
    (clojure.lang.BigInt/fromBigInteger (.not (java.math.BigInteger/valueOf (long x))))))


(defn bit-not' [x n]
  (clojure.lang.BigInt/fromBigInteger (.not (to-biginteger x) n)))


(defn bit-and' [x y]
  (if (and (instance? clojure.lang.BigInt x)
           (instance? clojure.lang.BigInt y))
    (if-let [xb (.bipart x)]
      (if-let [yb (.bipart y)]
        ;; x and y are both represented by BigInteger
        (clojure.lang.BigInt/fromBigInteger (.and xb yb))
        ;; x is BigInteger, but y is long
        (clojure.lang.BigInt/fromBigInteger (.and xb (java.math.BigInteger/valueOf (.lpart y)))))
      ;; else
      (if-let [yb (.bipart y)]
        ;; y is BigInteger, but x is long
        (clojure.lang.BigInt/fromBigInteger (.and (java.math.BigInteger/valueOf (.lpart x)) yb))
        ;; x and y are both represented by long
        (bigint (bit-and (.lpart x) (.lpart y)))))

    ;; else convert the non-BigInt values to BigInt, then handle it as
    ;; above.
    (bit-and' (bigint x) (bigint y))))


(defn bit-or' [x y]
  (if (and (instance? clojure.lang.BigInt x)
           (instance? clojure.lang.BigInt y))
    (if-let [xb (.bipart x)]
      (if-let [yb (.bipart y)]
        ;; x and y are both represented by BigInteger
        (clojure.lang.BigInt/fromBigInteger (.or xb yb))
        ;; x is BigInteger, but y is long
        (clojure.lang.BigInt/fromBigInteger (.or xb (java.math.BigInteger/valueOf (.lpart y)))))
      ;; else
      (if-let [yb (.bipart y)]
        ;; y is BigInteger, but x is long
        (clojure.lang.BigInt/fromBigInteger (.or (java.math.BigInteger/valueOf (.lpart x)) yb))
        ;; x and y are both represented by long
        (bigint (bit-or (.lpart x) (.lpart y)))))

    ;; else convert the non-BigInt values to BigInt, then handle it as
    ;; above.
    (bit-or' (bigint x) (bigint y))))


(defn bit-xor' [x y]
  (if (and (instance? clojure.lang.BigInt x)
           (instance? clojure.lang.BigInt y))
    (if-let [xb (.bipart x)]
      (if-let [yb (.bipart y)]
        ;; x and y are both represented by BigInteger
        (clojure.lang.BigInt/fromBigInteger (.xor xb yb))
        ;; x is BigInteger, but y is long
        (clojure.lang.BigInt/fromBigInteger (.xor xb (java.math.BigInteger/valueOf (.lpart y)))))
      ;; else
      (if-let [yb (.bipart y)]
        ;; y is BigInteger, but x is long
        (clojure.lang.BigInt/fromBigInteger (.xor (java.math.BigInteger/valueOf (.lpart x)) yb))
        ;; x and y are both represented by long
        (bigint (bit-xor (.lpart x) (.lpart y)))))

    ;; else convert the non-BigInt values to BigInt, then handle it as
    ;; above.
    (bit-xor' (bigint x) (bigint y))))


(defn pow [x n]
  (cond
    (zero? n) 1
    (= n 1) x
    :else
    (let [tmp (pow x (/ n 2))]
      (if (even? n)
        (*' tmp tmp)
        (*' tmp tmp x)))))


(comment

(source *')

(def two-to-the-64 (pow 2N 64))
two-to-the-64

(bit-shift-left 1 64)
(= two-to-the-64 (bit-shift-left' 1 64))
(= two-to-the-64 (bit-shift-left' 1N 64))
(= (*' 4 two-to-the-64) (bit-shift-left' (bit-shift-left' 1 64) 2))
(= (*' 4 two-to-the-64) (bit-shift-left' (bit-shift-left' 1N 64) 2))
(= (*' 10 two-to-the-64) (bit-shift-left' 10 64))
(= (*' 10 two-to-the-64) (bit-shift-left' 10N 64))

(= (bit-shift-right Long/MAX_VALUE 7)
   (bit-shift-right' Long/MAX_VALUE 7))

(= 1
   (bit-shift-right Long/MAX_VALUE 62)
   (bit-shift-right' Long/MAX_VALUE 62))

(= 0
   (bit-shift-right Long/MAX_VALUE 63)
   (bit-shift-right' Long/MAX_VALUE 63))

;; Note: for shift values larger than 63, bit-shift-right will use a
;; shift amount of (n % 64), I believe.  bit-shift-right' does _not_
;; do that.
(= 0
   (bit-shift-right' Long/MAX_VALUE 64))

(= 0
   (bit-shift-right' Long/MAX_VALUE 65))

(= 4
   (bit-shift-right' (*' 4 two-to-the-64) 64))

(= two-to-the-64
   (bit-shift-right' (*' two-to-the-64 two-to-the-64) 64))

Long/MAX_VALUE
(unchecked-multiply 2 Long/MAX_VALUE)

(= (*' 2 Long/MAX_VALUE)
   (bit-shift-left' Long/MAX_VALUE 1))

(= (bit-and Long/MAX_VALUE 17)
   (bit-and' Long/MAX_VALUE 17))

(= (bit-or Long/MAX_VALUE 17)
   (bit-or' Long/MAX_VALUE 17))

(= (bit-xor Long/MAX_VALUE 17)
   (bit-xor' Long/MAX_VALUE 17))

(= 0
   (bit-and' Long/MAX_VALUE two-to-the-64))

(= (+' Long/MAX_VALUE two-to-the-64)
   (bit-or' Long/MAX_VALUE two-to-the-64))

(= (+' Long/MAX_VALUE two-to-the-64)
   (bit-xor' Long/MAX_VALUE two-to-the-64))

(= 0
   (bit-and' (*' 2 Long/MAX_VALUE) two-to-the-64))

(= (+' (*' 2 Long/MAX_VALUE) two-to-the-64)
   (bit-or' (*' 2 Long/MAX_VALUE) two-to-the-64))

(= (+' (*' 2 Long/MAX_VALUE) two-to-the-64)
   (bit-xor' (*' 2 Long/MAX_VALUE) two-to-the-64))

(= two-to-the-64
   (bit-and' (*' 4 Long/MAX_VALUE) two-to-the-64))

(= (*' 4 Long/MAX_VALUE)
   (bit-or' (*' 4 Long/MAX_VALUE) two-to-the-64))

(= (-' (*' 4 Long/MAX_VALUE) two-to-the-64)
   (bit-xor' (*' 4 Long/MAX_VALUE) two-to-the-64))

(= (bit-not 1)
   (bit-not' 1))

(= (bit-not Long/MAX_VALUE)
   (bit-not' Long/MAX_VALUE))

(= (bit-not Long/MIN_VALUE)
   (bit-not' Long/MIN_VALUE))

  )
