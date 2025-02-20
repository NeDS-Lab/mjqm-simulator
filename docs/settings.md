
## Output columns
```toml
[output]
columns = [
  "cores[*]",         # these apply for each class (if the value is not specified, the default is used)
  "arrival.prob[*]",  # these apply for each class (if the value is not specified, the default is used)
  "service.mean[*]",  # these apply for each class (if the value is not specified, the default is used)
  "arrival.prob[50]", # thise apply for a specific class, identified by the cores value, or by name if defined
  "cores",            # these apply for default values (if any)
  "arrival.rate",     # these apply for default values (if any)
  "service.mean",     # these apply for default values (if any)
  "pivots",           # this ensures all pivot values are included
  "-pivots",          # this removes all pivot values
  "-Window Size",     # this removes a specific computed column
  "-*",               # this removes all columns (computed and pivots), useful to only choose a handful of columns
]
columns = [ # this is the default value
  "*",
  "pivots",
]
```
