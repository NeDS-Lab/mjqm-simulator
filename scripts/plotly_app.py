# Import packages

import dash_daq as daq
import plotly.express as px
from dash import (
    Dash,
    Input,
    Output,
    State,
    callback,
    dash_table,
    dcc,
    html,
    no_update,
)
from load_experiment_data import (
    load_experiment_data,
    load_experiments_list,
)
from plot_cells import prepare_cosmetics

base, available_experiments = load_experiments_list()

y_axis_mappings = dict(
    response_time=dict(
        column="RespTime Total",
        label="Response Time",
        class_column="T{} RespTime",
        uom=" [s]",
        per_class=True,
    ),
    waiting_time=dict(
        column="WaitTime Total",
        label="Waiting Time",
        class_column="T{} Waiting",
        uom=" [s]",
        per_class=True,
    ),
    # run_duration=dict(
    #     column="Run Duration",
    #     label="Simulation Run Duration",
    #     uom=" [s]",
    #     per_class=False,
    # ),
    wasted_servers=dict(
        column="Wasted Servers",
        label="Wasted Servers",
        uom="",
        per_class=False,
    ),
)


# Initialize the app
app = Dash("mjqm")
# App layout
app.layout = [
    html.H1(
        [
            "MJQM Simulation results ",
            html.Span("", id="title-slug"),
        ],
        id="title",
    ),
    html.Div(
        children=[
            html.Div(
                [
                    html.Div(
                        "Experiment",
                        style={
                            "margin-left": ".5em",
                        },
                    ),
                    dcc.Dropdown(
                        options=list(
                            map(
                                lambda f: str(f.relative_to(base)),
                                available_experiments,
                            )
                        ),
                        value=None,
                        placeholder="Select the experiment to analyse",
                        id="experiment-selection",
                        persistence=True,
                        persistence_type="session",
                        style={
                            "width": "99%",
                            # "display": "inline-block",
                            "margin-left": ".5em",
                        },
                    ),
                ],
                style={
                    "vertical-align": "middle",
                    "width": "39%",
                },
            ),
            daq.NumericInput(
                id="experiment-cores",
                min=1,
                max=10_000_000,
                value=2048,
                label="Simulated cores",
                persistence=True,
                persistence_type="session",
                style={
                    "margin-left": "1em",
                },
                size=100,
            ),
            dcc.Tabs(
                id="display-table",
                value="yes",
                children=[
                    dcc.Tab(
                        label="Show data",
                        value="yes",
                        style=dict(padding=".5em"),
                        selected_style=dict(padding=".5em"),
                    ),
                    dcc.Tab(
                        label="Hide data",
                        value="no",
                        style=dict(padding=".5em"),
                        selected_style=dict(padding=".5em"),
                    ),
                ],
                style={
                    "border": "thin lightgrey solid",
                    "margin-left": "1em",
                    # "overflowX": "scroll",
                    # "width": "49%",
                    # "float": "left",
                },
            ),
        ],
        style={
            "justifyContent": "flex-start",
            "display": "flex",
            "margin-bottom": "1em",
        },
    ),
    html.Div(
        [
            dcc.Loading(
                children=[
                    dash_table.DataTable(
                        data=None,
                        page_size=10,
                        id="experiment-data-table",
                        persistence=True,
                        persistence_type="session",
                        fixed_columns=dict(headers=True, data=2),
                        sort_action="native",
                        sort_mode="multi",
                        export_format="csv",
                        export_headers="names",
                        css=[
                            dict(
                                selector=".dash-spreadsheet.dash-freeze-left",
                                rule="max-width: 100%",
                            )
                        ],
                        style_table={"overflowX": "auto"},
                    ),
                ],
                target_components={
                    "experiment-data-table": "data",
                },
                overlay_style={"visibility": "visible", "filter": "blur(2px)"},
                type="circle",
                show_initially=False,
                parent_style={
                    # "border-left": "1px solid #222222",
                    # "justifyContent": "flex-start",
                    # "display": "flex",
                    "width": "98%",
                    "margin-left": "1em",
                },
            ),
        ],
        id="table-container",
        style={"margin-bottom": "1em"},
    ),
    html.Div(
        [
            dcc.Tabs(
                id="y-axis-value",
                value="response_time",
                children=list(
                    map(
                        lambda x: dcc.Tab(label=x[1]["label"], value=x[0]),
                        y_axis_mappings.items(),
                    )
                ),
                style={
                    "border": "thin lightgrey solid",
                    "overflowX": "scroll",
                    "width": "49%",
                    "float": "left",
                },
            ),
            dcc.Tabs(
                id="y-axis-group",
                value="overall",
                children=[
                    dcc.Tab(label="Overall", value="overall"),
                    dcc.Tab(
                        label="Smallest Class",
                        value="smallest_class",
                        id="smallest-class-tab",
                    ),
                    dcc.Tab(
                        label="Biggest Class",
                        value="biggest_class",
                        id="biggest-class-tab",
                    ),
                    dcc.Tab(
                        label="Select Class",
                        value="select_class",
                        id="select-class-tab",
                        children=[
                            dcc.Dropdown(
                                options=[],
                                value=None,
                                placeholder="The class to analyse",
                                id="custom-class-selection",
                                persistence=True,
                                persistence_type="session",
                                style={
                                    "width": "69%",
                                    "float": "right",
                                },
                            ),
                        ],
                    ),
                ],
                style={
                    "border": "thin lightgrey solid",
                    "overflowX": "scroll",
                    "width": "49%",
                    "float": "right",
                },
            ),
        ]
    ),
    dcc.Loading(
        children=[
            dcc.Graph(
                figure={},
                id="plot-total-response-time",
                config={
                    # "fillFrame": True,
                    "displaylogo": False,
                },
                mathjax=True,
            ),
            # html.Br(),
            # dcc.Clipboard(target_id="structure"),
            # html.Pre(
            #     id="structure",
            #     style={
            #         "border": "thin lightgrey solid",
            #         "overflowX": "scroll",
            #         "height": "275px",
            #         "width": "100%",
            #     },
            # ),
        ],
        target_components={
            "plot-total-response-time": "figure",
        },
        overlay_style={"visibility": "visible", "filter": "blur(2px)"},
        type="circle",
        show_initially=False,
        parent_style={
            "position": "relative",
            "float": "inline-start",
            "width": "99%",
        },
    ),
]


@callback(
    Output("title-slug", "children"),
    Input("experiment-selection", "value"),
)
def update_title(experiment):
    if experiment is None:
        return None
    return "from " + experiment


@callback(
    Output("table-container", "style", allow_duplicate=True),
    Input("display-table", "value"),
    prevent_initial_call=True,
)
def update_table_view(display_table):
    if display_table == "yes":
        return dict(display="block")
    return dict(display="none")


@callback(
    Output("experiment-data-table", "data"),
    Output("table-container", "style", allow_duplicate=True),
    Output("experiment-cores", "value"),
    Output("experiment-cores", "disabled"),
    Input("experiment-selection", "value"),
    Input("experiment-cores", "value"),
    State("display-table", "value"),
    running=[
        (Output("experiment-selection", "disabled"), True, False),
    ],
    prevent_initial_call=True,
)
def update_table(experiment, cores, display_table):
    global dfs, Ts, exp, asymptotes, actual_util
    if experiment is None:
        return None, dict(display="none"), no_update, False
    results = load_experiment_data(experiment, n_cores=cores or 2048)
    if results is None:
        return None, dict(display="none"), no_update, False
    dfs, Ts, exp, asymptotes, actual_util = results
    if "cores" in dfs.columns:
        cores = dfs["cores"].max()
        cores_selectable = False
    else:
        cores = no_update
        cores_selectable = True
    return (
        dfs.to_dict("records"),
        dict(
            width="100%", display="block" if display_table == "yes" else "none"
        ),
        cores,
        not cores_selectable,
    )


@callback(
    Output("custom-class-selection", "options"),
    Output("custom-class-selection", "value"),
    Output("smallest-class-tab", "disabled", allow_duplicate=True),
    Output("biggest-class-tab", "disabled", allow_duplicate=True),
    Output("select-class-tab", "disabled", allow_duplicate=True),
    Input("experiment-data-table", "data"),
    prevent_initial_call=True,
)
def populate_classes_list(experiment):
    global dfs, Ts, exp, asymptotes, actual_util
    if experiment is None:
        return None, None, True, True, True
    return Ts, min(Ts), False, False, False


@callback(
    Output("y-axis-group", "value"),
    Output("smallest-class-tab", "disabled", allow_duplicate=True),
    Output("biggest-class-tab", "disabled", allow_duplicate=True),
    Output("select-class-tab", "disabled", allow_duplicate=True),
    Input("y-axis-value", "value"),
    prevent_initial_call=True,
)
def only_overall_value(y_axis):
    if y_axis is None:
        return no_update, False, False, False
    per_class = y_axis_mappings[y_axis]["per_class"]
    if not per_class:
        return "overall", True, True, True
    return no_update, False, False, False


@callback(
    Output("plot-total-response-time", "figure"),
    Output("plot-total-response-time", "style"),
    Input("experiment-data-table", "data"),
    Input("y-axis-value", "value"),
    Input("y-axis-group", "value"),
    Input("custom-class-selection", "value"),
    running=[
        (Output("experiment-selection", "disabled"), True, False),
    ],
)
def show_totresp(experiment, y_axis, y_group, selected_class):
    global dfs, Ts, exp, asymptotes, actual_util
    if experiment is None:
        return None, dict(visibility="hidden")

    label = y_axis_mappings[y_axis]["label"]
    uom = y_axis_mappings[y_axis]["uom"]
    per_class = y_axis_mappings[y_axis]["per_class"]

    if y_group == "overall" or not per_class:
        col = y_axis_mappings[y_axis]["column"]
        pass
    elif y_group == "smallest_class":
        col = y_axis_mappings[y_axis]["class_column"]
        col = col.format(min(Ts))
        label += " for the Smallest Class"
    elif y_group == "biggest_class":
        col = y_axis_mappings[y_axis]["class_column"]
        col = col.format(max(Ts))
        label += " for the Biggest Class"
    elif y_group == "select_class":
        col = y_axis_mappings[y_axis]["class_column"]
        col = col.format(selected_class)
        label += f" for Class {selected_class}"

    colors, marks, marks_plotly = prepare_cosmetics(dfs, exp)
    dfs.set_index(["label"], drop=False, inplace=True)
    fig = px.line(
        dfs[dfs["stable"]],
        "arrival.rate",
        col,
        color="label",
        line_group="label",
        symbol="label",
        hover_name="label",
        hover_data={
            "label": False,
            "arrival.rate": False,
            col: True,
        },
        log_x=True,
        log_y=True,
        title=f"Avg. {label} vs. Arrival Rate",
        color_discrete_map=colors.to_dict(),
        symbol_sequence=list(map(str, marks_plotly)),
        labels={
            "label": "Policy",
            "arrival.rate": "Arrival Rate [1/s]",
            col: label + uom,
        },
        template="plotly_white",
    )
    for idx in asymptotes.index:
        fig.add_vline(
            asymptotes[idx],
            name=f"{actual_util[idx]:.1f}%",
            legend="legend2",
            line=dict(dash="dot", width=1, color=colors[idx]),
            showlegend=True,
        )
    fig.update_layout(
        title=dict(xanchor="center", x=0.5, yanchor="top"),
        legend=dict(
            yanchor="top",
            y=0.99,
            xanchor="left",
            x=0.01,
            title=None,
        ),
        legend2=dict(
            yanchor="middle",
            y=0.5,
            xanchor="left",
            x=0.98,
            title=None,
        ),
        hoversubplots="axis",
        hovermode="x unified",
        # xaxis=dict(zerolinecolor="black"),
        # yaxis=dict(zerolinecolor="black"),
    )
    return fig, dict(visibility="visible")


# @app.callback(
#     Output("structure", "children"),
#     Input("plot-total-response-time", component_property="figure"),
# )
# def display_structure(fig_json):
#     return json.dumps(fig_json, indent=2)


# Run the app
if __name__ == "__main__":
    app.run(debug=True)
