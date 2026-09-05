=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.then -> if.then2, if.then -> if.else4, if.end -> if.then16, if.else -> if.end, if.then2 -> if.then7, if.end3 -> if.end, if.end8 -> if.end3, if.else9 -> if.end8, if.then12 -> if.end13, if.end13 -> if.end8, if.else14 -> if.end13, if.then16 -> if.then21, if.then16 -> if.else23, if.end17 -> if.then35, if.end17 -> if.else37, if.else18 -> if.end17, if.then21 -> if.then26, if.end22 -> if.end17, if.then26 -> if.then31, if.then26 -> if.else33, if.end27 -> if.end22, if.else28 -> if.end27, if.end32 -> if.end27, if.then35 -> if.then39, if.then35 -> if.else41, if.end36 -> while.cond, if.else37 -> if.end36, if.then39 -> if.then43, if.end40 -> if.end36, if.end44 -> if.end40, if.else45 -> if.end44
INSTRUMENTED: if.end -> if.else18, if.then2 -> if.else9, if.else4 -> if.end3, if.then7 -> if.then12, if.then7 -> if.else14, if.then21 -> if.else28, if.else23 -> if.end22, if.then31 -> if.end32, if.else33 -> if.end32, if.then39 -> if.else45, if.else41 -> if.end40, if.then43 -> if.end44
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.then2 : 0
if.then -> if.else4 : 0
if.end -> if.then16 : 0
if.end -> if.else18 : 100000000
if.else -> if.end : 0
if.then2 -> if.then7 : 0
if.then2 -> if.else9 : 0
if.end3 -> if.end : 0
if.else4 -> if.end3 : 0
if.then7 -> if.then12 : 200000000
if.then7 -> if.else14 : 0
if.end8 -> if.end3 : 200000000
if.else9 -> if.end8 : 0
if.then12 -> if.end13 : 0
if.end13 -> if.end8 : 0
if.else14 -> if.end13 : 0
if.then16 -> if.then21 : 0
if.then16 -> if.else23 : 0
if.end17 -> if.then35 : 0
if.end17 -> if.else37 : 0
if.else18 -> if.end17 : 0
if.then21 -> if.then26 : 0
if.then21 -> if.else28 : 1
if.end22 -> if.end17 : 0
if.else23 -> if.end22 : 0
if.then26 -> if.then31 : 0
if.then26 -> if.else33 : 0
if.end27 -> if.end22 : 100000000
if.else28 -> if.end27 : 0
if.then31 -> if.end32 : 99999999
if.end32 -> if.end27 : 0
if.else33 -> if.end32 : 0
if.then35 -> if.then39 : 0
if.then35 -> if.else41 : 0
if.end36 -> while.cond : 0
if.else37 -> if.end36 : 0
if.then39 -> if.then43 : 0
if.then39 -> if.else45 : 0
if.end40 -> if.end36 : 0
if.else41 -> if.end40 : 0
if.then43 -> if.end44 : 200000000
if.end44 -> if.end40 : 200000000
if.else45 -> if.end44 : 0
END_PROFILE
