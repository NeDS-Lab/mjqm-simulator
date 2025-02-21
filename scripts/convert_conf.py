import sys
from pathlib import Path


def class_def(tupl):
    return f"""
[[class]]
cores = {tupl[0].strip()}
arrival.prob = {tupl[1].strip()}
service.mean = {tupl[2].strip()}
"""


if __name__ == "__main__":
    name = sys.argv[1]
    if len(sys.argv) == 3:
        out = sys.argv[2]
        if out == "stdout" or out == "-":
            out = sys.stdout
        else:
            out = Path(out).open("w")
    else:
        out = (Path("Inputs") / (name + ".toml")).open("w")
    classes = Path("Inputs") / (name + ".txt")
    rates = Path("Inputs") / ("arrRate_" + name + ".txt")
    classes = classes.read_text().splitlines()
    rates = rates.read_text().strip()
    classes = [c.strip("()").split(",") for c in classes]
    classes = "".join([class_def(c) for c in classes])
    print(
        f"""identifier = "{name}"
events = 100000
repetitions = 10
cores = 50
policy = "smash"
smash.window = 1

arrival.distribution = "exponential"

service.distribution = "exponential"

{classes}

[[pivot]]
arrival.rate = {rates}
""",
        file=out,
    )
