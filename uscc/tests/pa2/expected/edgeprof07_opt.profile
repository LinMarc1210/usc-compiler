=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.end -> while.cond, if.else -> if.else6, if.then4 -> if.end5, if.end5 -> if.end, if.else6 -> if.then9, if.else6 -> if.else11, if.end10 -> if.end5, if.else11 -> if.else16, if.then14 -> if.end15, if.end15 -> if.end10, if.else16 -> if.then19, if.else16 -> if.else21, if.end20 -> if.end15, if.else21 -> if.then24, if.else21 -> if.else26, if.end25 -> if.end20, if.else26 -> if.then29, if.else26 -> if.else31, if.end30 -> if.end25
INSTRUMENTED: if.then -> if.end, if.else -> if.then4, if.then9 -> if.end10, if.else11 -> if.then14, if.then19 -> if.end20, if.then24 -> if.end25, if.then29 -> if.end30, if.else31 -> if.end30
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.end : 25000000
if.end -> while.cond : 0
if.else -> if.then4 : 25000000
if.else -> if.else6 : 0
if.then4 -> if.end5 : 0
if.end5 -> if.end : 175000000
if.else6 -> if.then9 : 0
if.else6 -> if.else11 : 0
if.then9 -> if.end10 : 25000000
if.end10 -> if.end5 : 0
if.else11 -> if.then14 : 25000000
if.else11 -> if.else16 : 0
if.then14 -> if.end15 : 0
if.end15 -> if.end10 : 125000000
if.else16 -> if.then19 : 0
if.else16 -> if.else21 : 0
if.then19 -> if.end20 : 25000000
if.end20 -> if.end15 : 0
if.else21 -> if.then24 : 0
if.else21 -> if.else26 : 0
if.then24 -> if.end25 : 25000000
if.end25 -> if.end20 : 75000000
if.else26 -> if.then29 : 0
if.else26 -> if.else31 : 0
if.then29 -> if.end30 : 25000000
if.end30 -> if.end25 : 50000000
if.else31 -> if.end30 : 25000000
END_PROFILE
