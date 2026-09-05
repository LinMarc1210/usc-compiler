=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.then -> if.else35, if.end -> while.cond, if.else -> if.then2, if.else -> if.else4, if.end3 -> if.end, if.else4 -> if.then6, if.else4 -> if.else8, if.then6 -> if.then23, if.then6 -> if.else25, if.end7 -> if.end3, if.else8 -> if.else12, if.then10 -> if.end11, if.end11 -> if.end7, if.else12 -> if.then14, if.end15 -> if.end11, if.else16 -> if.end15, if.then18 -> if.end19, if.end19 -> if.end15, if.else20 -> if.end19, if.end24 -> if.end7, if.then28 -> if.end29, if.end29 -> if.end3, if.else30 -> if.end29, if.then33 -> if.end34, if.end34 -> if.end, if.else35 -> if.then38, if.end39 -> if.end34, if.else40 -> if.end39
INSTRUMENTED: if.then -> if.then33, if.then2 -> if.then28, if.then2 -> if.else30, if.else8 -> if.then10, if.else12 -> if.else16, if.then14 -> if.then18, if.then14 -> if.else20, if.then23 -> if.end24, if.else25 -> if.end24, if.else35 -> if.else40, if.then38 -> if.end39
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.then33 : 8917749
if.then -> if.else35 : 0
if.end -> while.cond : 0
if.else -> if.then2 : 0
if.else -> if.else4 : 0
if.then2 -> if.then28 : 7792207
if.then2 -> if.else30 : 15411256
if.end3 -> if.end : 0
if.else4 -> if.then6 : 0
if.else4 -> if.else8 : 0
if.then6 -> if.then23 : 0
if.then6 -> if.else25 : 0
if.end7 -> if.end3 : 0
if.else8 -> if.then10 : 41298702
if.else8 -> if.else12 : 0
if.then10 -> if.end11 : 0
if.end11 -> if.end7 : 0
if.else12 -> if.then14 : 0
if.else12 -> if.else16 : 21038961
if.then14 -> if.then18 : 692640
if.then14 -> if.else20 : 7099567
if.end15 -> if.end11 : 0
if.else16 -> if.end15 : 0
if.then18 -> if.end19 : 0
if.end19 -> if.end15 : 0
if.else20 -> if.end19 : 0
if.then23 -> if.end24 : 13939394
if.end24 -> if.end7 : 0
if.else25 -> if.end24 : 14285714
if.then28 -> if.end29 : 0
if.end29 -> if.end3 : 0
if.else30 -> if.end29 : 0
if.then33 -> if.end34 : 0
if.end34 -> if.end : 0
if.else35 -> if.then38 : 0
if.else35 -> if.else40 : 41298702
if.then38 -> if.end39 : 28225108
if.end39 -> if.end34 : 0
if.else40 -> if.end39 : 41298702
END_PROFILE
