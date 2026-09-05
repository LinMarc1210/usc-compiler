=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.then -> if.then2, if.then -> if.else4, if.end -> if.then22, if.else -> if.end, if.then2 -> if.then8, if.end3 -> if.end, if.end9 -> if.end3, if.else10 -> if.end9, if.then15 -> if.end16, if.end16 -> if.end9, if.else17 -> if.end16, if.then22 -> if.then28, if.then22 -> if.else30, if.end23 -> if.then49, if.end23 -> if.else51, if.else24 -> if.end23, if.then28 -> if.then35, if.end29 -> if.end23, if.then35 -> if.then42, if.then35 -> if.else44, if.end36 -> if.end29, if.else37 -> if.end36, if.end43 -> if.end36, if.then49 -> if.then54, if.then49 -> if.else56, if.end50 -> while.cond, if.else51 -> if.end50, if.then54 -> if.then59, if.end55 -> if.end50, if.end60 -> if.end55, if.else61 -> if.end60
INSTRUMENTED: if.end -> if.else24, if.then2 -> if.else10, if.else4 -> if.end3, if.then8 -> if.then15, if.then8 -> if.else17, if.then28 -> if.else37, if.else30 -> if.end29, if.then42 -> if.end43, if.else44 -> if.end43, if.then54 -> if.else61, if.else56 -> if.end55, if.then59 -> if.end60
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.then2 : 0
if.then -> if.else4 : 0
if.end -> if.then22 : 0
if.end -> if.else24 : 100000000
if.else -> if.end : 0
if.then2 -> if.then8 : 0
if.then2 -> if.else10 : 0
if.end3 -> if.end : 0
if.else4 -> if.end3 : 0
if.then8 -> if.then15 : 200000000
if.then8 -> if.else17 : 0
if.end9 -> if.end3 : 200000000
if.else10 -> if.end9 : 0
if.then15 -> if.end16 : 0
if.end16 -> if.end9 : 0
if.else17 -> if.end16 : 0
if.then22 -> if.then28 : 0
if.then22 -> if.else30 : 0
if.end23 -> if.then49 : 0
if.end23 -> if.else51 : 0
if.else24 -> if.end23 : 0
if.then28 -> if.then35 : 0
if.then28 -> if.else37 : 1
if.end29 -> if.end23 : 0
if.else30 -> if.end29 : 0
if.then35 -> if.then42 : 0
if.then35 -> if.else44 : 0
if.end36 -> if.end29 : 100000000
if.else37 -> if.end36 : 0
if.then42 -> if.end43 : 99999999
if.end43 -> if.end36 : 0
if.else44 -> if.end43 : 0
if.then49 -> if.then54 : 0
if.then49 -> if.else56 : 0
if.end50 -> while.cond : 0
if.else51 -> if.end50 : 0
if.then54 -> if.then59 : 0
if.then54 -> if.else61 : 0
if.end55 -> if.end50 : 0
if.else56 -> if.end55 : 0
if.then59 -> if.end60 : 200000000
if.end60 -> if.end55 : 200000000
if.else61 -> if.end60 : 0
END_PROFILE
