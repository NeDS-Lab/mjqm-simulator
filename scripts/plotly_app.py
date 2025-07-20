# Import packages

import plotly.express as px
from dash import Dash, Input, Output, callback, dash_table, dcc, html, no_update
from load_experiment_data import (
    load_experiment_data,
    load_experiments_list,
)
from plot_cells import prepare_cosmetics

# Initialize the app
app = Dash("mjqm")

base, available_experiments = load_experiments_list()

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
                            "width": "89%",
                            "display": "inline-block",
                            "margin-left": ".5em",
                        },
                    ),
                ],
                style={
                    # "border-right": "3px solid #222222",
                    "vertical-align": "middle",
                    "width": "49%",
                    "float": "left",
                },
            ),
            html.Div(
                [
                    html.Div(
                        "System cores",
                        style={
                            "margin-left": ".5em",
                        },
                    ),
                    dcc.Input(
                        value=2048,
                        placeholder="Cores of the simulated system",
                        id="experiment-cores",
                        type="number",
                        persistence=True,
                        persistence_type="session",
                    ),
                ],
                style={
                    # "border-right": "3px solid #222222",
                    "vertical-align": "middle",
                    "width": "49%",
                    "float": "left",
                },
            ),
        ]
    ),
    dcc.Loading(
        children=[
            dash_table.DataTable(
                data=None,
                page_size=5,
                id="experiment-data-table",
                persistence=True,
                persistence_type="session",
                fixed_columns=dict(headers=True, data=2),
                sort_action="native",
                sort_mode="multi",
                export_format="csv",
                export_headers="names",
                style_table={"max-width": "100%"},
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
            "float": "left",
            "width": "94%",
            "margin-left": "1em",
        },
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
    Output("experiment-data-table", "data"),
    Output("experiment-data-table", "style_table"),
    Output("experiment-cores", "value"),
    Output("experiment-cores", "disabled"),
    Input("experiment-selection", "value"),
    Input("experiment-cores", "value"),
    running=[
        (Output("experiment-selection", "disabled"), True, False),
    ],
)
def update_table(experiment, cores):
    global dfs, Ts, exp, asymptotes, actual_util
    if experiment is None:
        return None, dict(visibility="hidden"), no_update, False
    results = load_experiment_data(experiment, n_cores=cores or 2048)
    if results is None:
        return None, dict(visibility="hidden"), no_update, False
    dfs, Ts, exp, asymptotes, actual_util = results
    if "cores" in dfs.columns:
        cores = dfs["cores"].max()
        cores_selectable = False
    else:
        cores = no_update
        cores_selectable = True
    return (
        dfs.to_dict("records"),
        dict(width="100%"),
        cores,
        not cores_selectable,
    )


@callback(
    Output("plot-total-response-time", "figure"),
    Output("plot-total-response-time", "style"),
    Input("experiment-data-table", "data"),
    running=[
        (Output("experiment-selection", "disabled"), True, False),
    ],
)
def show_totresp(experiment):
    global dfs, Ts, exp, asymptotes, actual_util
    if experiment is None:
        return None, dict(visibility="hidden")

    colors, marks, marks_plotly = prepare_cosmetics(dfs, exp)
    dfs.set_index(["label"], drop=False, inplace=True)
    fig = px.line(
        dfs[dfs["stable"]],
        "arrival.rate",
        "RespTime Total",
        color="label",
        line_group="label",
        symbol="label",
        hover_name="label",
        hover_data={
            "label": False,
            "arrival.rate": False,
            "RespTime Total": True,
        },
        log_x=True,
        log_y=True,
        title="Avg. Overall Response Time vs. Arrival Rate",
        color_discrete_map=colors.to_dict(),
        symbol_sequence=list(map(str, marks_plotly)),
        labels={
            "label": "Policy",
            "arrival.rate": "Arrival Rate [1/s]",
            "RespTime Total": "Avg. Response Time [s]",
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
