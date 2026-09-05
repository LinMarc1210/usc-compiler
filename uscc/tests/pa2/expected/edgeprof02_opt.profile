=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, while.body -> if.else, if.end -> while.cond, if.else -> if.else4, if.then2 -> if.end3, if.end3 -> if.end, if.else4 -> if.then6, if.else4 -> if.else8, if.end7 -> if.end3, if.else8 -> if.else12, if.then10 -> if.end11, if.end11 -> if.end7, if.else12 -> if.then14, if.else12 -> if.else16, if.end15 -> if.end11, if.else16 -> if.then18, if.else16 -> if.else20, if.end19 -> if.end15, if.else20 -> if.then22, if.else20 -> if.else24, if.end23 -> if.end19
INSTRUMENTED: if.then -> if.end, if.else -> if.then2, if.then6 -> if.end7, if.else8 -> if.then10, if.then14 -> if.end15, if.then18 -> if.end19, if.then22 -> if.end23, if.else24 -> if.end23
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.else : 0
if.then -> if.end : 25000000
if.end -> while.cond : 0
if.else -> if.then2 : 25000000
if.else -> if.else4 : 0
if.then2 -> if.end3 : 0
if.end3 -> if.end : 175000000
if.else4 -> if.then6 : 0
if.else4 -> if.else8 : 0
if.then6 -> if.end7 : 25000000
if.end7 -> if.end3 : 0
if.else8 -> if.then10 : 25000000
if.else8 -> if.else12 : 0
if.then10 -> if.end11 : 0
if.end11 -> if.end7 : 125000000
if.else12 -> if.then14 : 0
if.else12 -> if.else16 : 0
if.then14 -> if.end15 : 25000000
if.end15 -> if.end11 : 0
if.else16 -> if.then18 : 0
if.else16 -> if.else20 : 0
if.then18 -> if.end19 : 25000000
if.end19 -> if.end15 : 75000000
if.else20 -> if.then22 : 0
if.else20 -> if.else24 : 0
if.then22 -> if.end23 : 25000000
if.end23 -> if.end19 : 50000000
if.else24 -> if.end23 : 25000000
END_PROFILE
