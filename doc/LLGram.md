1 <prog> -> <?php declare(strict_types=1); <statement-list> <prog-end>
2 <prog-end> -> ε
3 <prog-end> -> ?>

4 <statement-list> -> <statement> <statement-list>
5 <statement-list> -> ε

6 <statement> -> var_id = <expression>;
7 <statement> -> return <expression>;
8 <statement> -> token_id(<argument-list>);
9 <statement> -> while(<expression>){<statement-list>}
10 <statement> -> if (<expression>) {<statement-list>} else {<statement-list>}
11 <statement> -> function token_id(<argument-list>) : type {<statement-list>}

12 <expression> -> int
14 <expression> -> float
15 <expression> -> string
16 <expression> -> var_id
16 <expression> -> (<expression>)
17 <expression> -> <expression> * <expression>
19 <expression> -> <expression> / <expression>
20 <expression> -> <expression> - <expression>
21 <expression> -> <expression> + <expression>
22 <expression> -> <expression> . <expression>
23 <expression> -> <expression> < <expression>
24 <expression> -> <expression> > <expression>
25 <expression> -> <expression> <= <expression>
26 <expression> -> <expression> >= <expression>
27 <expression> -> <expression> === <expression>
28 <expression> -> <expression> !== <expression>
29 <expression> -> token_id(<argument-list>)

30 <argument-list> -> <expression> <argument-next>
31 <argument-list> -> type <expression> <argument-next>
32 <argument-list> -> ε
33 <argument-next> -> ε
34 <argument-next> -> ,<expression> <argument-next>
35 <argument-next> -> ,type <expression> <argument-next5