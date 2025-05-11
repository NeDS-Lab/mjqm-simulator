#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Jan  7 16:21:22 2024

@author: dilettaolliaro
"""

from pathlib import Path
import re

import matplotlib
import matplotlib.pyplot as plt
import pandas as pd
from scipy.signal import savgol_filter

################################ KNOWN POLICIES ################################
policies_keys = [
    "smash",
    "fifo",
    "most server first",
    "server filling memoryful",
    "back filling",
]
policies_labels = [
    "SMASH w/ $w = {0}$",
    "First-In First-Out",
    "Most Server First",
    "Server Filling",
    "Back Filling",
]
policies = dict(zip(policies_keys, policies_labels))


################################ PANDAS AND PLOT CONFIGS ################################
policies_dtype = pd.api.types.CategoricalDtype(categories=policies_keys, ordered=True)
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
    index_asyms = [13, 52, 32, 27, 29, 34, 41]

    ys_bigResp = [7000, 5000, 3000, 500, 700, 1000, 2000]
    ys_resp = [2500, 750, 450, 15, 40, 100, 250]

    """wins = [-3, -2, 0, 1, 5]
    index_asyms = [13, 52, 32, 27, 34]

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
        # index_asyms = [39, 12, 31, 33, 37, 38, 13, 14]
        # ys_bigResp = [550, 400, 7, 15, 40, 100, 170, 200]
        # ys_resp = [70, 50, 4, 6, 8, 10, 25, 35]

        wins = [-3, -2, 0, 1, 2, 5, 10]
        index_asyms = [16, 39, 11, 30, 33, 35, 37]
        ys_bigResp = [550, 350, 400, 7, 15, 40, 100]
        ys_resp = [40, 30, 50, 2, 5, 8, 11]

        """wins = [-3, -2, 0, 1, 5]
        types = [0 for i in range(len(wins))]
        index_asyms = [16, 39, 11, 30, 35]
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
        index_asyms = [41, 41, 19, 26, 36, 40]
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

        index_asyms = [5]
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


folder = cell
folder = "253970"
folder = f"Results/{folder}"
dir = Path(folder)
filenames = list(dir.glob("*.csv"))
dfs = []


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


for f in filenames:
    df = pd.read_csv(f, delimiter=";")
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

    dfs.append(df)

df = None

defined_asymptotes = [
    float(dfs[i]["arrival.rate"][min(i, len(dfs[i]["arrival.rate"]) - 1)])
    for i, x in enumerate(index_asyms)
]

dfs = pd.concat(dfs)
types = {}
drops = []
Ts = set()
for column in dfs.columns:
    if "policy" == column or "label" == column or "Stability Check" in column:
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
print(dfs.head())

idx = ["label", "arrival.rate"]
dfs.sort_values(
    by=idx,
    inplace=True,
    ignore_index=True,
)
dfs.set_index(idx, drop=False, inplace=True)
exp = dfs.index.names.difference(["arrival.rate"])
if len(exp) == 1:
    exp = exp[0]
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

asymptotes = dfs.groupby(level=exp).apply(
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
    serTimes = [asymp_row[f"T{T} RespTime"] - asymp_row[f"T{T} Waiting"] for T in Ts]
    for t in range(len(Ts)):
        summ_util += asymptote * Ps[t] * serTimes[t] * Ts[t] * (1 / n)
    actual_util[idx] = summ_util * 100.0

############################### TOTAL RESPONSE TIME ##########################################

plt.figure(dpi=1200)
plt.rc("font", **{"family": "serif", "serif": ["Palatino"]})
plt.rc("text", usetex=True)
matplotlib.rcParams["font.size"] = fsize
matplotlib.rcParams["xtick.major.pad"] = 8
matplotlib.rcParams["ytick.major.pad"] = 8
fix, ax = plt.subplots(figsize=tuplesize)

i = 0
for idx, df_select in dfs.groupby(level=exp):
    f, i = i, i + 1
    x_data = df_select["arrival.rate"][df_select["stable"]]
    y_data = df_select["RespTime Total"][df_select["stable"]]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=str(idx), ls=st, lw=line_size)

    if (cell == "cellA" and wins[f] != 0) or cell == "cellB":
        plt.text(
            x=xs[f],
            y=ys_resp[f],
            s=f"{actual_util[idx]:.1f}\\%",
            rotation=0,
            c=cols[f],
            fontsize=tick_size,
            weight="extra bold",
        )
        plt.axvline(x=asymptotes[idx], color=cols[f], linestyle="dotted", lw=asym_size)

ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Response Time $\\quad[$s$]$", fontsize=label_size)
ax.set_title("Avg. Overall Response Time vs. Arrival Rate", fontsize=title_size)
plt.xscale("log")
plt.yscale("log")
plt.ylim(ylims_totResp[0], ylims_totResp[1])
plt.xlim(xlims[0], xlims[1])
# plt.yticks(fontsize=tick_size)
# plt.xticks(fontsize=tick_size, which = 'minor')
ax.tick_params(axis="both", which="major", labelsize=tick_size, pad=l_pad)
ax.tick_params(axis="both", which="minor", labelsize=tick_size, pad=l_pad)

ax.legend(fontsize=legend_size, loc=legend_locs[0])

ax.grid()
plt.savefig(f"{folder}/lambdasVsTotRespTime-{cell}_{n}.pdf", bbox_inches="tight")

############################### SMALL CLASS RESPONSE TIME ##########################################

plt.figure(dpi=1200)
plt.rc("font", **{"family": "serif", "serif": ["Palatino"]})
plt.rc("text", usetex=True)
matplotlib.rcParams["font.size"] = fsize
fix, ax = plt.subplots(figsize=tuplesize)

i = 0
for idx, df_select in dfs.groupby(level=exp):
    f, i = i, i + 1
    x_data = df_select["arrival.rate"][df_select["stable"]]
    y_data = df_select[f"T{min(Ts)} RespTime"][df_select["stable"]]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=str(idx), ls=st, lw=line_size)

    if (cell == "cellA" and wins[f] != 0) or cell == "cellB":
        plt.text(
            x=xs[f],
            y=ys_resp[f],
            s=f"{actual_util[idx]:.1f}\\%",
            rotation=0,
            c=cols[f],
            fontsize=tick_size,
            weight="extra bold",
        )
        plt.axvline(x=asymptotes[idx], color=cols[f], linestyle="dotted", lw=asym_size)

ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Response Time $\\quad[$s$]$", fontsize=label_size)
ax.set_title(
    "Avg. Response Time for the Smallest Class vs. Arrival Rate",
    fontsize=title_size,
)
plt.xscale("log")
plt.yscale("log")
plt.ylim(ylims_smallResp[0], ylims_smallResp[1])
plt.xlim(xlims[0], xlims[1])
# plt.yticks(fontsize=tick_size)
# plt.xticks(fontsize=tick_size)
ax.tick_params(axis="both", which="major", labelsize=tick_size, pad=l_pad)
ax.tick_params(axis="both", which="minor", labelsize=tick_size, pad=l_pad)
# ax.legend(fontsize = legend_size, loc = legend_locs[1])
ax.grid()
plt.savefig(f"{folder}/lambdasVsSmallRespTime-{cell}_{n}.pdf", bbox_inches="tight")


############################### BIG CLASS RESPONSE TIME ##########################################


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

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=str(idx), ls=st, lw=line_size)

    if (cell == "cellA" and wins[f] != 0) or cell == "cellB":
        plt.text(
            x=xs[f],
            y=ys_bigResp[f],
            s=f"{actual_util[idx]:.1f}\\%",
            rotation=0,
            c=cols[f],
            fontsize=tick_size,
            weight="extra bold",
        )
        plt.axvline(x=asymptotes[idx], color=cols[f], linestyle="dotted", lw=asym_size)

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
plt.savefig(f"{folder}/lambdasVsBigRespTime-{cell}_{n}.pdf", bbox_inches="tight")


############################### TOTAL WAITING TIME ##########################################

plt.figure(dpi=1200)
plt.rc("font", **{"family": "serif", "serif": ["Palatino"]})
plt.rc("text", usetex=True)
matplotlib.rcParams["font.size"] = fsize
fix, ax = plt.subplots(figsize=tuplesize)

i = 0
for idx, df_select in dfs.groupby(level=exp):
    f, i = i, i + 1
    x_data = df_select["arrival.rate"][df_select["stable"]]
    y_data = df_select["WaitTime Total"][df_select["stable"]]
    y_interp = savgol_filter(y_data, 3, 2)

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=str(idx), ls=st, lw=line_size)

    if (cell == "cellA" and wins[f] != 0) or cell == "cellB":
        plt.axvline(x=asymptotes[idx], color=cols[f], linestyle="dotted", lw=asym_size)

ax.set_xlabel("Arrival Rate $\\quad[$s$^{-1}]$", fontsize=label_size)
ax.set_ylabel("Avg. Waiting Time $\\quad[$s$]$", fontsize=label_size)
ax.set_title("Avg. Overall Waiting Time vs. Arrival Rate", fontsize=title_size)
plt.xscale("log")
plt.yscale("log")
plt.ylim(ylims_totWait[0], ylims_totWait[1])
plt.xlim(xlims[0], xlims[1])
ax.tick_params(axis="both", which="major", labelsize=tick_size, pad=l_pad)
ax.tick_params(axis="both", which="minor", labelsize=tick_size, pad=l_pad)
# plt.yticks(fontsize=tick_size)
# plt.xticks(fontsize=tick_size)
# ax.legend(fontsize = legend_size, loc = legend_locs[3])


ax.grid()
plt.savefig(f"{folder}/lambdasVstotWaitTime-{cell}_{n}.pdf", bbox_inches="tight")


############################### SMALL CLASS WAITING TIME ##########################################

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

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=str(idx), ls=st, lw=line_size)

    # util = round(actual_util[idx]*100, 1)
    # plt.text(x = xs[f], y = ys[f], s = f'{util}\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
    if (cell == "cellA" and wins[f] != 0) or cell == "cellB":
        plt.axvline(x=asymptotes[idx], color=cols[f], linestyle="dotted", lw=asym_size)

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
plt.savefig(f"{folder}/lambdasVsSmallWaitTime-{cell}_{n}.pdf", bbox_inches="tight")


############################### BIG CLASS WAITING TIME ##########################################


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

    ax.scatter(x_data, y_data, color=cols[f], marker=markers[f], s=marker_size)
    ax.plot(x_data, y_interp, color=cols[f], label=str(idx), ls=st, lw=line_size)

    # util = round(actual_util[idx]*100, 1)
    # plt.text(x = xs[f], y = ys[f], s = f'{util}\%' , rotation=0, c = cols[f], fontsize = tick_size, weight= 'extra bold')
    if (cell == "cellA" and wins[f] != 0) or cell == "cellB":
        plt.axvline(x=asymptotes[idx], color=cols[f], linestyle="dotted", lw=asym_size)

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
plt.savefig(f"{folder}/lambdasVsBigWaitTime-{cell}_{n}.pdf", bbox_inches="tight")

######################################################################################
