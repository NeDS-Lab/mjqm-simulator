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
    classes_file = Path("Inputs") / (name + ".txt")
    rates = classes_file.parent / ("arrRate_" + classes_file.stem + ".txt")
    classes = classes_file.read_text().splitlines()
    rates = rates.read_text().strip()
    classes = [c.strip("()").split(",") for c in classes]
    classes = "".join([class_def(c) for c in classes])
    print(
        f"""identifier = "{classes_file.stem}"
events = 100000
repetitions = 10
cores = 50
policy = "smash"
smash.window = 1

arrival.distribution = "exponential"

service.distribution = "exponential"

{classes}

[[pivot]]
policy = [
    "fifo",
    "back filling",
    "server filling memoryful",
    "most server first",
    { name = "smash", window = 2 },
    { name = "smash", window = 5 },
    { name = "smash", window = 10 },
]
arrival.rate = {rates}
""",
        file=out,
    )
