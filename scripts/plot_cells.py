#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Jan  7 16:21:22 2024

@author: dilettaolliaro
"""

import math
import re
import sys
from pathlib import Path

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy.signal import savgol_filter

################################ KNOWN POLICIES ################################
policies_keys = [
    "smash",
    "fifo",
    "most server first",
    "server filling",
    "server filling memoryful",
    "back filling",
]
policies_labels = [
    "SMASH w/ $w = {0}$",
    "First-In First-Out",
    "Most Server First",
    "Server Filling",
    "Server Filling",
    "Back Filling",
]
policies = dict(zip(policies_keys, policies_labels))


################################ PANDAS AND PLOT CONFIGS ################################
policies_dtype = pd.api.types.CategoricalDtype(
    categories=policies_keys, ordered=True
)
stability_check_mapping = {
    "0": True,
    "1": False,
}  # we invert them because the column actually means "warning"

fsize = 150
legend_size = 200
label_size = 220
title_size = 195
tuplesize = (90, 70)
marker_size = 70**2
line_size = 25
tick_size = 180
l_pad = 40
asym_size = 20

cols = [
    "black",
    "peru",
    "darkorange",
    "royalblue",
    "crimson",
    "purple",
    "darkgreen",
    "pink",
]
markers = ["P", "o", "v", "s", "X", "D", "H", "<", ">"]
styles = ["solid", "dashdot", "dotted", "dashed", (0, (3, 5, 1, 5, 1, 5))]

cell = "cellA"
n = 4096
nClasses = 29

cell = "cellB"
n = 2048
nClasses = 26

inter_points = 10
# -3 : backfilling
# -2 : server filling
# 0: Most Server First
# 1: First-In First-Out
# i: SMASH w/ w=i

if cell == "cellB":
    wins = [-3, -2, 0, 1, 2, 5, 10]

    ys_bigResp = [7000, 5000, 3000, 500, 700, 1000, 2000]
    ys_resp = [2500, 750, 450, 15, 40, 100, 250]

    """wins = [-3, -2, 0, 1, 5]

    ys_bigResp = [700, 6000, 4700, 1000, 3000]
    ys_resp = [800, 550, 350, 7, 40]"""

    xs = [5.4, 5.4, 5.4, 5.4, 5.4, 5.4, 5.4]
    legend_locs = [
        "upper left",
        "upper left",
        "lower left",
        "upper left",
        "upper left",
        "lower right",
    ]

    ylims_totResp = [5, 10000]
    ylims_totWait = [0.01, 10**5]
    ylims_bigResp = [100, 10000]
    ylims_smallResp = [1, 10000]
    ylims_smallWait = [0.01, 10000]

    ylims = [0.1, 10**5]
    ylims_v2 = [0.1, 10**5]

    xlims = [0.01, 10]
    st = styles[0]

elif cell == "cellA":
    if n == 4096:
        # wins = [-2, 0, 1, 2, 5, 10, 50, 100]
        # ys_bigResp = [550, 400, 7, 15, 40, 100, 170, 200]
        # ys_resp = [70, 50, 4, 6, 8, 10, 25, 35]

        wins = [-3, -2, 0, 1, 2, 5, 10]
        ys_bigResp = [550, 350, 400, 7, 15, 40, 100]
        ys_resp = [40, 30, 50, 2, 5, 8, 11]

        """wins = [-3, -2, 0, 1, 5]
        types = [0 for i in range(len(wins))]
        ys_bigResp = [650, 550, 400, 7, 40]
        ys_resp = [100, 70, 50, 4, 8]"""

        xs = [725 for w in wins]
        # xs = [602, 739, 561, 614, 667, 711]

        ylims_totResp = [1, 100]
        ylims_totWait = [10**-3, 100]
        ylims_bigResp = [1, 1000]
        ylims_smallResp = [1, 100]
        ylims_smallWait = [10**-3, 100]
        legend_locs = [
            "upper left",
            "upper left",
            "lower left",
            "upper left",
            "upper left",
            "lower center",
        ]
        ylims = [1, 10**4]
        ylims_v2 = [0.001, 1000]

        xlims = [100, 10**3]
        st = styles[0]
    elif n == 3072:
        wins = [-2, 0, 1, 2, 5, 10]
        ys_bigResp = [800, 500, 20, 60, 120, 240]
        ys_resp = [10000, 6000, 200, 400, 800, 1400]

        xs = [370 for w in wins]
        # xs = [602, 739, 561, 614, 667, 711]

        ylims_totResp = [1, 100]
        ylims_totWait = [10**-3, 100]
        ylims_bigResp = [1, 1000]
        ylims_smallResp = [1, 100]
        ylims_smallWait = [10**-3, 100]

        ylims = [1, 10**4]
        ylims_v2 = [0.001, 1000]

        xlims = [100, 10**3]
        st = styles[0]
    elif n == 2048:
        wins = [0]

        ys_bigResp = [800]
        ys_resp = [1000]

        xs = [50 for w in wins]
        # xs = [602, 739, 561, 614, 667, 711]

        ylims_totResp = [0.001, 100]
        ylims_totWait = [0.001, 100]
        ylims_bigResp = [0.001, 1000]
        ylims_smallResp = [0.001, 100]
        ylims_smallWait = [0.001, 100]

        ylims = [0.001, 10**4]
        ylims_v2 = [0.001, 1000]

        xlims = [1, 500]
        xlims_big = [1, 30]
        st = styles[0]


def load_csv_filenames():
    results = Path("Results")
    # get folder from first argument or user input
    if len(sys.argv) > 1:
        folder = sys.argv[1]
    else:
        # list folders in Results directory
        print("Known folders:")
        for f in list(results.glob("*/")):
            print("-", f.stem)
        folder = input("Enter folder to read results from: ")
    folder = results / folder
    filenames = list(folder.glob("*.csv"))
    if not filenames:
        print(f"No CSV files found in {folder}", file=sys.stderr)
        exit(1)
    return folder, filenames


def row_label(row, win):
    if row["policy"] == "smash":
        if "policy.window" in row:
            return policies[row["policy"]].format(row["policy.window"])
        elif "smash.window" in row:
            return policies[row["policy"]].format(row["smash.window"])
        else:
            return policies[row["policy"]].format(win)
    else:
        return policies[row["policy"]]

required_columns = set(["policy", "arrival.rate", "Utilisation"])

def read_csv(f: Path):
    df = pd.read_csv(f, delimiter=";")
    if not all(column in df.columns for column in required_columns):
        print(f"Missing columns in {f}: {required_columns - df.columns}", file=sys.stderr)
        return None
    win = None
    if match := re.match(r"Win(?P<win>-?\d+)", f.stem):
        win = int(match.group("win"))
    df["label"] = df.apply(row_label, axis=1, args=(win,))
    actual_check = False
    stability_columns = []
    for column in df.columns:
        if "policy" == column:
            df[column] = df[column].astype(policies_dtype)
        elif "Stability Check" in column:
            df[column] = df[column].map(stability_check_mapping).astype(bool)
            if "Stability Check" == column:
                actual_check = True
            else:
                stability_columns.append(column)
    if not actual_check:
        df["Stability Check"] = df[stability_columns].all(axis=1)
    return df


def concat_csv_files(filenames: list[Path]):
    dfs = []
    for f in filenames:
        df = read_csv(f)
        if df is None:
            continue
        dfs.append(df)
    if not dfs:
        return None
    return pd.concat(dfs)


def clean_dfs(dfs):
    types = {}
    drops = []
    Ts = set()
    for column in dfs.columns:
        if "policy" == column:
            pass
        elif "label" == column:
            pass
        elif "Stability Check" in column:
            pass
        elif (
            "ConfInt" not in column
            and "Unnamed" not in column
            and not column.endswith(".window")
        ):
            types[column] = float
        else:
            drops.append(column)
        if match := re.match(r"T(?P<T>\d+)", column):
            Ts.add(int(match.group("T")))
    Ts = sorted(list(Ts))
    print(Ts)
    dfs = dfs.drop(columns=drops)
    dfs = dfs.astype(types)

    idx = ["label", "arrival.rate"]
    dfs.sort_values(
        by=idx,
        inplace=True,
        ignore_index=True,
    )
    dfs.set_index(idx, drop=False, inplace=True)
    dfs.sort_index(inplace=True)

    exp = dfs.index.names.difference(["arrival.rate"])
    if len(exp) == 1:
        exp = exp[0]

    return dfs, Ts, exp


def compute_stability(dfs, exp):
    arr_rate_increase = dfs.groupby(level=exp)["arrival.rate"].transform(
        lambda x: x.rolling(2).sem()
    )
    util_increase = dfs.groupby(level=exp)["Utilisation"].transform(
        lambda x: x.rolling(2).sem()
    )
    util_increase_ratio = arr_rate_increase / util_increase
    util_increase_ratio.name = "Utilisation Increase Ratio"
    dfs = pd.concat([dfs, util_increase_ratio], axis=1)
    divergence = dfs.groupby(level=exp)["Utilisation Increase Ratio"].transform(
        lambda x: x.rolling(2).apply(lambda x: abs(1.0 - x.iloc[1] / x.iloc[0]))
    )
    stable = dfs["Stability Check"] & (divergence.fillna(0) < 0.01)
    stable.name = "stable"
    dfs = pd.concat([dfs, stable], axis=1)
    return dfs


def compute_utilisation(dfs, Ts, exp):
    asymptotes = dfs.groupby(
        level=exp
    ).apply(
        lambda x: x["arrival.rate"]
        .shift(
            1, fill_value=x["arrival.rate"].max()
        )  # this shift makes the "minimum" extraction do what we want
        .where(~x["stable"], x["arrival.rate"].max())
        .min()  # keep the maximum (known) arrival rate where the system is still stable
    )

    actual_util = pd.Series(pd.NA, index=asymptotes.index)
    for idx, df_select in dfs.groupby(level=exp):
        summ_util = 0
        asymptote = asymptotes[idx]
        asymp_row = df_select.loc[idx, asymptote]
        Ps = [asymp_row[f"T{T} lambda"] / asymp_row["arrival.rate"] for T in Ts]
        serTimes = [
            asymp_row[f"T{T} RespTime"] - asymp_row[f"T{T} Waiting"] for T in Ts
        ]
        for t in range(len(Ts)):
            summ_util += asymptote * Ps[t] * serTimes[t] * Ts[t] * (1 / n)
        actual_util[idx] = summ_util * 100.0

    return asymptotes, actual_util


folder, filenames = load_csv_filenames()
dfs = concat_csv_files(filenames)
if dfs is None:
    print("No data found", file=sys.stderr)
    exit(1)
dfs, Ts, exp = clean_dfs(dfs)
dfs = compute_stability(dfs, exp)
asymptotes, actual_util = compute_utilisation(dfs, Ts, exp)


############################### PLOT UTILITIES ###############################


def prepare_cosmetics(dfs, exp):
    policy_groups = dfs.groupby(level=exp).groups.keys()
    # per each group, create a df with colors and markers
    colors = pd.Series(
        dtype=pd.api.types.CategoricalDtype(categories=cols, ordered=True),
        name="color",
    )
    marks = pd.Series(
        dtype=pd.api.types.CategoricalDtype(categories=markers, ordered=True),
        name="marker",
    )
    i = 0
    for group in policy_groups:
        colors[group] = cols[i]
        marks[group] = markers[i]
        i += 1

    return colors, marks


colors, marks = prepare_cosmetics(dfs, exp)


def add_legend(ax, legend):
    if legend:
        if isinstance(legend, tuple) or isinstance(legend, list):
            legend = legend[0]
            ncol = legend[1]
        else:
            ncol = 1
        ax.legend(fontsize=legend_size, loc=legend, ncol=ncol)


def compute_limits(ax, ylims, ns):
    ax.set_yscale("log")
    if ylims:
        ax.set_ylim(*ylims)
    ymin, ymax = ax.get_ylim()
    skip = (16 - ns) // 2

    ax.set_xscale("log")
    ax.set_xmargin(0)
    print("xbounds =", ax.get_xbound())
    xmin, xmax = ax.get_xbound()
    ax.set_xlim(xmin, 10 ** math.ceil(math.log10(xmax)))
    print("xbounds =", ax.get_xbound())

    return np.geomspace(ymin, ymax, num=16, endpoint=False)[-skip::-1]


############################# TOTAL RESPONSE TIME #############################


def plot_total_response_time(
    folder,
    dfs,
    exp,
    actual_util,
    asymptotes,
    ylims=None,
    legend=None,
    util_percentages=True,
):
    plt.figure(dpi=1200)
    plt.rc("font", **{"family": "serif", "serif": ["Palatino"]})
    plt.rc("text", usetex=True)
    matplotlib.rcParams["font.size"] = fsize
    matplotlib.rcParams["xtick.major.pad"] = 8
    matplotlib.rcParams["ytick.major.pad"] = 8
    fix, ax = plt.subplots(figsize=tuplesize)

    policy_groups = dfs.groupby(level=exp)
    for idx, df_select in policy_groups:
        x_data = df_select["arrival.rate"][df_select["stable"]]
        y_data = df_select["RespTime Total"][df_select["stable"]]
        y_interp = savgol_filter(y_data, 3, 2)

        ax.scatter(
            x_data, y_data, color=colors[idx], marker=marks[idx], s=marker_size
        )
        ax.plot(
            x_data,
            y_interp,
            color=colors[idx],
            label=str(idx),
            ls=st,
            lw=line_size,
        )

    ys = compute_limits(ax, ylims, policy_groups.ngroups)
    i = 0
    for idx, df_select in policy_groups:
        f, i = i, i + 1
        if util_percentages:
            plt.text(
                x=xs[f],
                y=ys[f],
                s=f"{actual_util[idx]:.1f}\\%",
                rotation=0,
                c=colors[idx],
                fontsize=tick_size,
                weight="extra bold",
            )
        plt.axvline(
            x=asymptotes[idx],
            color=colors[idx],
            linestyle="dotted",
            lw=asym_size,
        )

    ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
    ax.set_ylabel("Avg. Response Time $\\quad[$s$]$", fontsize=label_size)
    ax.set_title(
        "Avg. Overall Response Time vs. Arrival Rate", fontsize=title_size
    )
    ax.tick_params(axis="both", which="major", labelsize=tick_size, pad=l_pad)
    ax.tick_params(axis="both", which="minor", labelsize=tick_size, pad=l_pad)
    add_legend(ax, legend)
    ax.grid()
    rt_f = folder / "RespTime"
    rt_f.mkdir(parents=True, exist_ok=True)
    plt.savefig(rt_f / "lambdasVsTotRespTime.pdf", bbox_inches="tight")
    plt.savefig(rt_f / "lambdasVsTotRespTime.png", bbox_inches="tight")
    plt.close()


plot_total_response_time(
    folder,
    dfs,
    exp,
    actual_util,
    asymptotes,
    ylims=ylims_totResp,
    legend="upper left",
)


########################## SMALL CLASS RESPONSE TIME ##########################


def plot_class_response_time(
    folder,
    dfs,
    exp,
    T,
    actual_util,
    asymptotes,
    ylims=None,
    legend=None,
    util_percentages=True,
):
    plt.figure(dpi=1200)
    plt.rc("font", **{"family": "serif", "serif": ["Palatino"]})
    plt.rc("text", usetex=True)
    matplotlib.rcParams["font.size"] = fsize
    fix, ax = plt.subplots(figsize=tuplesize)

    policy_groups = dfs.groupby(level=exp)
    for idx, df_select in policy_groups:
        x_data = df_select["arrival.rate"][df_select["stable"]]
        y_data = df_select[f"T{T} RespTime"][df_select["stable"]]
        y_interp = savgol_filter(y_data, 3, 2)

        ax.scatter(
            x_data, y_data, color=colors[idx], marker=marks[idx], s=marker_size
        )
        ax.plot(
            x_data,
            y_interp,
            color=colors[idx],
            label=str(idx),
            ls=st,
            lw=line_size,
        )

    ys = compute_limits(ax, ylims, policy_groups.ngroups)
    i = 0
    for idx, df_select in policy_groups:
        f, i = i, i + 1
        if (cell == "cellA" and wins[f] != 0) or cell == "cellB":
            if util_percentages:
                plt.text(
                    x=xs[f],
                    y=ys[f],
                    s=f"{actual_util[idx]:.1f}\\%",
                    rotation=0,
                    c=colors[idx],
                    fontsize=tick_size,
                    weight="extra bold",
                )
            plt.axvline(
                x=asymptotes[idx],
                color=colors[idx],
                linestyle="dotted",
                lw=asym_size,
            )

    ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
    ax.set_ylabel("Avg. Response Time $\\quad[$s$]$", fontsize=label_size)
    ax.set_title(
        f"Avg. Response Time for Class ${T}$ vs. Arrival Rate",
        fontsize=title_size,
    )
    ax.tick_params(axis="both", which="major", labelsize=tick_size, pad=l_pad)
    ax.tick_params(axis="both", which="minor", labelsize=tick_size, pad=l_pad)

    add_legend(ax, legend)

    ax.grid()
    rt_f = folder / "RespTime"
    rt_f.mkdir(parents=True, exist_ok=True)
    plt.savefig(rt_f / f"lambdasVsT{T}RespTime.pdf", bbox_inches="tight")
    plt.savefig(rt_f / f"lambdasVsT{T}RespTime.png", bbox_inches="tight")
    plt.close()


for T in Ts:
    plot_class_response_time(folder, dfs, exp, T, actual_util, asymptotes)


########################### BIG CLASS RESPONSE TIME ###########################


plt.figure(dpi=1200)
plt.rc("font", **{"family": "serif", "serif": ["Palatino"]})
plt.rc("text", usetex=True)
matplotlib.rcParams["font.size"] = fsize
fix, ax = plt.subplots(figsize=tuplesize)

i = 0
for idx, df_select in dfs.groupby(level=exp):
    f, i = i, i + 1
    x_data = df_select["arrival.rate"][df_select["stable"]]
    y_data = df_select[f"T{max(Ts)} RespTime"][df_select["stable"]]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(
        x_data, y_data, color=colors[idx], marker=marks[idx], s=marker_size
    )
    ax.plot(
        x_data, y_interp, color=colors[idx], label=str(idx), ls=st, lw=line_size
    )

    if (cell == "cellA" and wins[f] != 0) or cell == "cellB":
        plt.text(
            x=xs[f],
            y=ys_bigResp[f],
            s=f"{actual_util[idx]:.1f}\\%",
            rotation=0,
            c=colors[idx],
            fontsize=tick_size,
            weight="extra bold",
        )
        plt.axvline(
            x=asymptotes[idx],
            color=colors[idx],
            linestyle="dotted",
            lw=asym_size,
        )

ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Response Time $\\quad[$s$]$", fontsize=label_size)
ax.set_title(
    "Avg. Response Time for the Biggest Class vs. Arrival Rate",
    fontsize=title_size,
)
plt.xscale("log")
plt.yscale("log")
plt.ylim(ylims_bigResp[0], ylims_bigResp[1])
plt.xlim(xlims[0], xlims[1])
ax.tick_params(axis="both", which="major", labelsize=tick_size, pad=l_pad)
ax.tick_params(axis="both", which="minor", labelsize=tick_size, pad=l_pad)
# plt.yticks(fontsize=tick_size)
# plt.xticks(fontsize=tick_size)
# ax.legend(fontsize = legend_size, loc = legend_locs[2], ncols=2)


ax.grid()
plt.savefig(
    folder / f"lambdasVsBigRespTime-{cell}_{n}.pdf", bbox_inches="tight"
)


############################## TOTAL WAITING TIME ##############################


def plot_total_waiting_time(
    folder,
    dfs,
    exp,
    actual_util,
    asymptotes,
    ylims=None,
    legend=None,
    util_percentages=True,
):
    plt.figure(dpi=1200)
    plt.rc("font", **{"family": "serif", "serif": ["Palatino"]})
    plt.rc("text", usetex=True)
    matplotlib.rcParams["font.size"] = fsize
    matplotlib.rcParams["xtick.major.pad"] = 8
    matplotlib.rcParams["ytick.major.pad"] = 8
    fix, ax = plt.subplots(figsize=tuplesize)

    policy_groups = dfs.groupby(level=exp)
    for idx, df_select in policy_groups:
        x_data = df_select["arrival.rate"][df_select["stable"]]
        y_data = df_select["WaitTime Total"][df_select["stable"]]
        y_interp = savgol_filter(y_data, 3, 2)

        ax.scatter(
            x_data, y_data, color=colors[idx], marker=marks[idx], s=marker_size
        )
        ax.plot(
            x_data,
            y_interp,
            color=colors[idx],
            label=str(idx),
            ls=st,
            lw=line_size,
        )

    ys = compute_limits(ax, ylims, policy_groups.ngroups)
    i = 0
    for idx, df_select in policy_groups:
        f, i = i, i + 1
        if util_percentages:
            plt.text(
                x=xs[f],
                y=ys[f],
                s=f"{actual_util[idx]:.1f}\\%",
                rotation=0,
                c=colors[idx],
                fontsize=tick_size,
                weight="extra bold",
            )
        plt.axvline(
            x=asymptotes[idx],
            color=colors[idx],
            linestyle="dotted",
            lw=asym_size,
        )

    ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
    ax.set_ylabel("Avg. Waiting Time $\\quad[$s$]$", fontsize=label_size)
    ax.set_title(
        "Avg. Overall Waiting Time vs. Arrival Rate", fontsize=title_size
    )
    ax.tick_params(axis="both", which="major", labelsize=tick_size, pad=l_pad)
    ax.tick_params(axis="both", which="minor", labelsize=tick_size, pad=l_pad)
    add_legend(ax, legend)
    ax.grid()
    rt_f = folder / "WaitTime"
    rt_f.mkdir(parents=True, exist_ok=True)
    plt.savefig(rt_f / "lambdasVsTotWaitTime.pdf", bbox_inches="tight")
    plt.savefig(rt_f / "lambdasVsTotWaitTime.png", bbox_inches="tight")
    plt.close()


plot_total_waiting_time(
    folder,
    dfs,
    exp,
    actual_util,
    asymptotes,
    ylims=ylims_totWait,
    legend="upper left",
)


########################### SMALL CLASS WAITING TIME ###########################


plt.figure(dpi=1200)
plt.rc("font", **{"family": "serif", "serif": ["Palatino"]})
plt.rc("text", usetex=True)
matplotlib.rcParams["font.size"] = fsize
fix, ax = plt.subplots(figsize=tuplesize)

i = 0
for idx, df_select in dfs.groupby(level=exp):
    f, i = i, i + 1
    x_data = df_select["arrival.rate"][df_select["stable"]]
    y_data = df_select[f"T{min(Ts)} Waiting"][df_select["stable"]]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(
        x_data, y_data, color=colors[idx], marker=marks[idx], s=marker_size
    )
    ax.plot(
        x_data, y_interp, color=colors[idx], label=str(idx), ls=st, lw=line_size
    )

    # util = round(actual_util[idx]*100, 1)
    # plt.text(x = xs[f], y = ys[f], s = f'{util}\%' , rotation=0, c = colors[idx], fontsize = tick_size, weight= 'extra bold')
    if (cell == "cellA" and wins[f] != 0) or cell == "cellB":
        plt.axvline(
            x=asymptotes[idx],
            color=colors[idx],
            linestyle="dotted",
            lw=asym_size,
        )

ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Waiting Time $\\quad[$s$]$", fontsize=label_size)
ax.set_title(
    "Avg. Waiting Time for the Smallest Class vs. Arrival Rate",
    fontsize=title_size,
)
plt.xscale("log")
plt.yscale("log")
plt.ylim(ylims_smallWait[0], ylims_smallWait[1])
plt.xlim(xlims[0], xlims[1])
# plt.yticks(fontsize=tick_size)
# plt.xticks(fontsize=tick_size)
ax.tick_params(axis="both", which="major", labelsize=tick_size, pad=l_pad)
ax.tick_params(axis="both", which="minor", labelsize=tick_size, pad=l_pad)
# ax.legend(fontsize = legend_size, loc = legend_locs[4])


ax.grid()
plt.savefig(
    folder / f"lambdasVsSmallWaitTime-{cell}_{n}.pdf", bbox_inches="tight"
)


############################ BIG CLASS WAITING TIME ############################


plt.figure(dpi=1200)
plt.rc("font", **{"family": "serif", "serif": ["Palatino"]})
plt.rc("text", usetex=True)
matplotlib.rcParams["font.size"] = fsize
fix, ax = plt.subplots(figsize=tuplesize)

i = 0
for idx, df_select in dfs.groupby(level=exp):
    f, i = i, i + 1
    x_data = df_select["arrival.rate"][df_select["stable"]]
    y_data = df_select[f"T{max(Ts)} Waiting"][df_select["stable"]]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(
        x_data, y_data, color=colors[idx], marker=marks[idx], s=marker_size
    )
    ax.plot(
        x_data, y_interp, color=colors[idx], label=str(idx), ls=st, lw=line_size
    )

    # util = round(actual_util[idx]*100, 1)
    # plt.text(x = xs[f], y = ys[f], s = f'{util}\%' , rotation=0, c = colors[idx], fontsize = tick_size, weight= 'extra bold')
    if (cell == "cellA" and wins[f] != 0) or cell == "cellB":
        plt.axvline(
            x=asymptotes[idx],
            color=colors[idx],
            linestyle="dotted",
            lw=asym_size,
        )

ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Waiting Time $\\quad[$s$]$", fontsize=label_size)
ax.set_title(
    "Avg. Waiting Time for the Biggest Class vs. Arrival Rate",
    fontsize=title_size,
)
plt.xscale("log")
plt.yscale("log")
plt.ylim(ylims_v2[0], ylims_v2[1])
plt.xlim(xlims[0], xlims[1])
ax.tick_params(axis="both", which="major", labelsize=tick_size, pad=l_pad)
ax.tick_params(axis="both", which="minor", labelsize=tick_size, pad=l_pad)
# plt.yticks(fontsize=tick_size)
# plt.xticks(fontsize=tick_size)
# ax.legend(fontsize = legend_size, loc = legend_locs[5])


ax.grid()
plt.savefig(
    folder / f"lambdasVsBigWaitTime-{cell}_{n}.pdf", bbox_inches="tight"
)


################################################################################
