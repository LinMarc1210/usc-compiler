=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> for.cond : 1
for.cond -> for.body : 200
for.cond -> for.end : 1
for.body -> if.then : 4
for.body -> if.end : 196
for.step -> for.cond : 200
if.then -> if.end : 4
if.end -> for.step : 200
END_PROFILE
