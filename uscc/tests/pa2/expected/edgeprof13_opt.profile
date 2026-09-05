=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.then -> if.else43, if.end -> while.cond, if.else -> if.then3, if.else -> if.else5, if.end4 -> if.end, if.else5 -> if.then8, if.else5 -> if.else10, if.then8 -> if.then29, if.then8 -> if.else31, if.end9 -> if.end4, if.else10 -> if.else15, if.then13 -> if.end14, if.end14 -> if.end9, if.else15 -> if.then18, if.end19 -> if.end14, if.else20 -> if.end19, if.then23 -> if.end24, if.end24 -> if.end19, if.else25 -> if.end24, if.end30 -> if.end9, if.then35 -> if.end36, if.end36 -> if.end4, if.else37 -> if.end36, if.then41 -> if.end42, if.end42 -> if.end, if.else43 -> if.then47, if.end48 -> if.end42, if.else49 -> if.end48
INSTRUMENTED: if.then -> if.then41, if.then3 -> if.then35, if.then3 -> if.else37, if.else10 -> if.then13, if.else15 -> if.else20, if.then18 -> if.then23, if.then18 -> if.else25, if.then29 -> if.end30, if.else31 -> if.end30, if.else43 -> if.else49, if.then47 -> if.end48
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.then41 : 8917749
if.then -> if.else43 : 0
if.end -> while.cond : 0
if.else -> if.then3 : 0
if.else -> if.else5 : 0
if.then3 -> if.then35 : 7792207
if.then3 -> if.else37 : 15411256
if.end4 -> if.end : 0
if.else5 -> if.then8 : 0
if.else5 -> if.else10 : 0
if.then8 -> if.then29 : 0
if.then8 -> if.else31 : 0
if.end9 -> if.end4 : 0
if.else10 -> if.then13 : 41298702
if.else10 -> if.else15 : 0
if.then13 -> if.end14 : 0
if.end14 -> if.end9 : 0
if.else15 -> if.then18 : 0
if.else15 -> if.else20 : 21038961
if.then18 -> if.then23 : 692640
if.then18 -> if.else25 : 7099567
if.end19 -> if.end14 : 0
if.else20 -> if.end19 : 0
if.then23 -> if.end24 : 0
if.end24 -> if.end19 : 0
if.else25 -> if.end24 : 0
if.then29 -> if.end30 : 13939394
if.end30 -> if.end9 : 0
if.else31 -> if.end30 : 14285714
if.then35 -> if.end36 : 0
if.end36 -> if.end4 : 0
if.else37 -> if.end36 : 0
if.then41 -> if.end42 : 0
if.end42 -> if.end : 0
if.else43 -> if.then47 : 0
if.else43 -> if.else49 : 41298702
if.then47 -> if.end48 : 28225108
if.end48 -> if.end42 : 0
if.else49 -> if.end48 : 41298702
END_PROFILE
