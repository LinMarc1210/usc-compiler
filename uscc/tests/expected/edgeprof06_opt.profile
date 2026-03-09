=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, while.body -> if.then, if.end -> if.end3, if.then2 -> if.end3, if.end3 -> if.end7, if.then6 -> if.end7, if.end7 -> if.end11, if.then10 -> if.end11, if.end11 -> if.end15, if.then14 -> if.end15, if.end15 -> if.end19, if.then18 -> if.end19, if.end19 -> if.end23, if.then22 -> if.end23, if.end23 -> if.end27, if.then26 -> if.end27, if.end27 -> if.end31, if.then30 -> if.end31, if.end31 -> if.end35, if.then34 -> if.end35, if.end35 -> while.cond
INSTRUMENTED: while.body -> if.end, if.then -> if.end, if.end -> if.then2, if.end3 -> if.then6, if.end7 -> if.then10, if.end11 -> if.then14, if.end15 -> if.then18, if.end19 -> if.then22, if.end23 -> if.then26, if.end27 -> if.then30, if.end31 -> if.then34
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 0
while.body -> if.end : 198000000
if.then -> if.end : 2000000
if.end -> if.then2 : 2000000
if.end -> if.end3 : 0
if.then2 -> if.end3 : 0
if.end3 -> if.then6 : 2000000
if.end3 -> if.end7 : 0
if.then6 -> if.end7 : 0
if.end7 -> if.then10 : 2000000
if.end7 -> if.end11 : 0
if.then10 -> if.end11 : 0
if.end11 -> if.then14 : 2000000
if.end11 -> if.end15 : 0
if.then14 -> if.end15 : 0
if.end15 -> if.then18 : 2000000
if.end15 -> if.end19 : 0
if.then18 -> if.end19 : 0
if.end19 -> if.then22 : 2000000
if.end19 -> if.end23 : 0
if.then22 -> if.end23 : 0
if.end23 -> if.then26 : 2000000
if.end23 -> if.end27 : 0
if.then26 -> if.end27 : 0
if.end27 -> if.then30 : 2000000
if.end27 -> if.end31 : 0
if.then30 -> if.end31 : 0
if.end31 -> if.then34 : 2000000
if.end31 -> if.end35 : 0
if.then34 -> if.end35 : 0
if.end35 -> while.cond : 0
END_PROFILE
