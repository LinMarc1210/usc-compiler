=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 3000
while.cond -> while.end : 1
while.body -> if.then : 1500
while.body -> if.else : 1500
if.then -> while.cond8 : 1500
if.end -> if.then20 : 3
if.end -> if.end21 : 2997
if.else -> while.cond2 : 1500
while.cond2 -> while.body4 : 4500000
while.cond2 -> while.end5 : 1500
while.body4 -> while.cond2 : 4500000
while.end5 -> if.end : 1500
while.cond8 -> while.body11 : 4500000
while.cond8 -> while.end12 : 1500
while.body11 -> while.cond8 : 4500000
while.end12 -> if.end : 1500
if.then20 -> if.end21 : 3
if.end21 -> while.cond : 3000
END_PROFILE
