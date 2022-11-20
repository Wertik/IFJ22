```
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

12 <expression> -> var_type
13 <expression> -> var_id
14 <expression> -> (<expression>)
15 <expression> -> <expression> * <expression>
16 <expression> -> <expression> / <expression>
17 <expression> -> <expression> - <expression>
18 <expression> -> <expression> + <expression>
19 <expression> -> <expression> . <expression>
21 <expression> -> <expression> < <expression>
22 <expression> -> <expression> > <expression>
23 <expression> -> <expression> <= <expression>
24 <expression> -> <expression> >= <expression>
25 <expression> -> <expression> === <expression>
26 <expression> -> <expression> !== <expression>
27 <expression> -> token_id(<argument-list>)

28 <argument-list> -> <expression> <argument-next-list>
29 <argument-list> -> type <expression> <argument-next-list>
30 <argument-list> -> ε
31 <argument-next-list> -> ,<expression> <argument-next-list>
32 <argument-next-list> -> ,type <expression> <argument-next-list>
33 <argument-next-list> -> ε
```