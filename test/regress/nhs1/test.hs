succ 9 + max 5 4 + 1 

(succ 9) + (max 5 4) + 1

max 100 101

doubleUs 4 9

doubleUs 2.3 34.2

doubleUs 28 88 + doubleMe 123

doubleSmallNumber 7

doubleSmallNumber' 7

[1,2,3,4] ++ [9,10,11,12] 

"hello" ++ " " ++ "world"

['w','o'] ++ ['o','t']  

'A':" SMALL CAT"

5:[1,2,3,4,5] 

"Steve Buscemi" !! 6  

[9.4,33.2,96.2,11.2,23.25] !! 1 

let b = [[1,2,3,4],[5,3,3,3],[1,2,2,3,4],[1,2,3]]

b

[1..20]

['a'..'z']

[2,4..20]

[0.1, 0.3 .. 1]

take 10 (repeat 5)

[x*2 | x <- [1..10]]

[x*2 | x <- [1..10], x*2 >= 12]

[ x | x <- [50..100], x `mod` 7 == 3]

[ x | x <- [10..20], x /= 13, x /= 15, x /= 19]

[ x*y | x <- [2,5,10], y <- [8,10,11], x*y > 50]

let nouns = ["prince","frog","pope"]
let adjectives = ["lazy","grouchy","scheming"]  
[adjective ++ " " ++ noun | adjective <- adjectives, noun <- nouns]

length' [1,2,3]

let xxs = [[1,3,5,2,3,1,2,4,5],[1,2,3,4,5,6,7,8,9],[1,2,4,2,1,6,3,1,3,2,3,6]]
[ [ x | x <- xs, even x ] | xs <- xxs] 

fst (8,11)

zip [1,2,3,4,5] [5,5,5,5,5]

let rightTriangles = [ (a,b,c) | c <- [1..10], b <- [1..c], a <- [1..b], a^2 + b^2 == c^2]
rightTriangles

removeNonUppercase "AaBbCc"

addVectors (1,2) (3,4)

first'' (2,3,4)

tell [1]
tell [1,2]
tell [1,2,3]

capital "FooBar"

bmiTell 23.0

bmiTell' 140.0 29.0

initials "Charles" "Smith"

cylinder 1.0 2.0

[if 5 > 3 then "Woo" else "Boo", if 'a' > 'b' then "Foo" else "Bar"]

4 * (let a = 9 in a + 1) + 2

[let a = 5 in a]

max (1 + 1) (2 + 3)

(max 4) 5

replicate' 4 5

take' 3 [4,5,6,7,8]

reverse' [1,2,3]

take' 3 (repeat' 5)

zip' [1,2,3] [4,5,6]

elem' 2 [1,2,3]

quicksort [4,1,4,7,3,2]

let multTwoWithNine = multThree 9 

multTwoWithNine 2 3

let multWithEighteen = multTwoWithNine 2

multWithEighteen 10

divideByTen 1000

applyTwice (* 2) 3

applyTwice (multThree 2 2) 9

applyTwice (3:) [1]  

zipWith' (+) [4,2,5,6] [2,6,2,3] 

zipWith' max [6,3,2,1] [7,3,1,5]

zipWith' (++) ["foo ", "bar ", "baz "] ["fighters", "hoppers", "aldrin"]

zipWith' (*) (replicate 5 2) [1..]

zipWith' (zipWith' (*)) [[1,2,3],[3,5,6],[2,3,4]] [[3,2,2],[3,4,5],[5,4,3]]

flip' zip [1,2,3,4,5] "hello"

zipWith (flip' div) [2,2..] [10,8,6,4,2]

map' (+3) [1,5,3,1,6] 

map' (++ "!") ["BIFF", "BANG", "POW"]

map' (replicate 3) [3..6]

map' (map (^2)) [[1,2],[3,4,5,6],[7,8]]

map' fst [(1,2),(3,5),(6,3),(2,6),(2,5)]

filter (>3) [1,5,3,2,1,6,4,3,2,1]

filter (==3) [1,2,3,4,5]

filter even [1..10] 

let notNull x = not (null x) in filter notNull [[1,2,3],[],[3,4,5],[2,2],[],[],[]]

filter (`elem` ['a'..'z']) "BeCaUsE I aM diFfeRent"

filter (`elem` ['A'..'Z']) "difFrent CasES" 

quicksort' [4,1,4,7,3,2]

sum (takeWhile (<10000) (filter odd (map (^2) [1..])))

sum (takeWhile (<10000) [n^2 | n <- [1..], odd (n^2)])

chain 30

zipWith (\a b -> (a * 30 + 3) / b) [5,4,3,2,1] [1,2,3,4,5] 

map (\(a,b) -> a + b) [(1,2),(3,5),(6,3),(2,6),(2,5)]  

sum' [1,2,3,4]

sum $ filter (> 10) $ map (*2) [2..10]

and $ map (>4) [5,6,7,8]

and $ map (==4) [4,4,4,3,4]

take 10 $ iterate (*2) 1

sum $ takeWhile (<10000) $ map (^3) [1..]

filter odd . map (^2) $ [1..10]

map (^2) $ [1..10]

map (^2) [1..10]

map (\x -> negate (abs x)) [5,-3,-6,7,-3,2,-19,24]

map (\xs -> negate (sum (tail xs))) [[1..5],[3..6],[1..7]]

map (negate . sum . tail) [[1..5],[3..6],[1..7]]

oddSquareSum

and $ map (>4) [5,6,7,8]

surface $ Circle1 10 20 10 

surface $ Rectangle1 0 0 100 100

-- problems start here

nudge (Circle (Point 34 34) 10) 5 10

nudge (baseRect 40 100) 60 23

Car {company="Ford", model="Mustang", year=1967}

let stang = Car {company="Ford", model="Mustang", year=1967}

tellCar stang

tellCar (Car "Ford" "Mustang" 1967)

TestVector 3 5 8 `vplus` TestVector 9 2 8 

TestVector 3 5 8 `vplus` TestVector 9 2 8 `vplus` TestVector 0 2 3

TestVector 3 9 7 `vectMult` 10 

TestVector 4 9 5 `scalarMult` TestVector 9.0 2.0 4.0 

TestVector 2 9 3 `vectMult` (TestVector 3 7 8 `scalarMult` TestVector 9 2 4)

show Wednesday 

