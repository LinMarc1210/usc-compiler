=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.else : 100000000
if.then -> if.end : 100000000
if.end -> if.then7 : 66666667
if.end -> if.else9 : 133333333
if.else -> if.end : 100000000
if.then7 -> if.end8 : 66666667
if.end8 -> if.then17 : 40000000
if.end8 -> if.else19 : 160000000
if.else9 -> if.end8 : 133333333
if.then17 -> if.end18 : 40000000
if.end18 -> if.then27 : 28571429
if.end18 -> if.else29 : 171428571
if.else19 -> if.end18 : 160000000
if.then27 -> if.end28 : 28571429
if.end28 -> if.then37 : 18181819
if.end28 -> if.else39 : 181818181
if.else29 -> if.end28 : 171428571
if.then37 -> if.end38 : 18181819
if.end38 -> while.cond : 200000000
if.else39 -> if.end38 : 181818181
END_PROFILE
