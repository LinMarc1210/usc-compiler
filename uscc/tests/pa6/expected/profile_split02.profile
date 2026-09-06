=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 50
while.cond -> while.end : 1
while.body -> while.cond3 : 50
while.cond3 -> while.body6 : 1000
while.cond3 -> while.end7 : 50
while.body6 -> while.cond3 : 1000
while.end7 -> while.cond : 50
END_PROFILE
