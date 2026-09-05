=== OPTIMIZED EDGE PROFILING (MST) ===
EDGE_PROFILE: main
SPANNING_TREE: entry -> while.cond, while.cond -> while.body, while.cond -> while.end, if.then -> if.end, if.end -> if.else9, if.else -> if.end, if.then7 -> if.end8, if.end8 -> if.else19, if.else9 -> if.end8, if.then17 -> if.end18, if.end18 -> if.else29, if.else19 -> if.end18, if.then27 -> if.end28, if.end28 -> if.else39, if.else29 -> if.end28, if.then37 -> if.end38, if.end38 -> while.cond, if.else39 -> if.end38
INSTRUMENTED: while.body -> if.then, while.body -> if.else, if.end -> if.then7, if.end8 -> if.then17, if.end18 -> if.then27, if.end28 -> if.then37
EDGES:
entry -> while.cond : 0
while.cond -> while.body : 0
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.else : 100000000
if.then -> if.end : 0
if.end -> if.then7 : 66666667
if.end -> if.else9 : 0
if.else -> if.end : 0
if.then7 -> if.end8 : 0
if.end8 -> if.then17 : 40000000
if.end8 -> if.else19 : 0
if.else9 -> if.end8 : 0
if.then17 -> if.end18 : 0
if.end18 -> if.then27 : 28571429
if.end18 -> if.else29 : 0
if.else19 -> if.end18 : 0
if.then27 -> if.end28 : 0
if.end28 -> if.then37 : 18181819
if.end28 -> if.else39 : 0
if.else29 -> if.end28 : 0
if.then37 -> if.end38 : 0
if.end38 -> while.cond : 0
if.else39 -> if.end38 : 0
END_PROFILE
