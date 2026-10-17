# Python Export Script

[Back to the main guide](../README.md)

`generate_plots.py` trains a random forest regressor, writes five charts and a summary, and prints status and metrics. It can run directly or as a child process of the Qt application.

```bash
python scripts/generate_plots.py
```

Run this command from the repository root after installing the [Python dependencies](../README.md#run-the-python-workflows).

## Imports and paths

`from __future__ import print_function` selects function-style printing for older Python syntax compatibility; this does not establish compatibility for the script's dependencies. `os` handles filesystem paths and directory creation.

`matplotlib.use('Agg')` selects a noninteractive renderer before importing `pyplot`. Charts can therefore be exported without opening a plotting window. NumPy generates synthetic plot data, pandas processes tables, and scikit-learn supplies encoding, splitting, regression, and evaluation.

| Constant | Resolution |
| --- | --- |
| `ROOT_DIR` | Absolute parent of the directory containing the script. |
| `DATASET_PATH` | `ROOT_DIR/logatta.csv`. |
| `OUTPUT_DIR` | `ROOT_DIR/qt4_app/generated`. |

These paths do not depend on the terminal's current directory.

## Every function

| Function | Implementation and result |
| --- | --- |
| `ensure_output_dir()` | Checks `os.path.isdir(OUTPUT_DIR)` and calls `os.makedirs(...)` if needed. |
| `load_dataframe()` | Reads the CSV; maps travel, marital status, overtime, gender, and target; drops missing rows; copies the result; label-encodes remaining object columns. Returns `(df, target_col)`. |
| `train_model(df, target_col)` | Uses all non-target columns, including `EmployeeNumber`; converts them to NumPy arrays; splits 80/20 with seed 42; trains a 100-tree `RandomForestRegressor` with OOB scoring enabled. Computes test MSE and R² and returns `(model, feature_cols, mse, r2_value)`. |
| `save_feature_importance_plot(model, feature_cols)` | Pairs forest importances with column names, sorts them ascending, and saves a horizontal bar chart. Returns the top three as a descending Series. |
| `save_distribution_plot(df, target_col)` | Counts binary labels, renames them Accepted/Rejected, and saves a bar chart. |
| `save_age_histogram(df, target_col)` | Groups by outcome and overlays age histograms with 15 bins and opacity 0.6. |
| `save_line_plot()` | Plots `sin(x)` for 100 evenly spaced values from 0 to 10. This uses synthetic data. |
| `save_scatter_plot()` | Generates 50 seeded random points, with independently generated sizes and colors; adds a color bar and saves the figure. Its local `df` does not overwrite the analysis dataframe. |
| `save_summary(df, mse, r2_value, top_features)` | Counts accepted/rejected records; formats the model name, MSE, R², and top feature scores; writes `summary.txt`; returns its lines for console printing. |
| `main()` | Calls the functions in order, exports all artifacts, and prints the dataset path, output directory, summary, and completion message. |

The final `if __name__ == '__main__': main()` starts the workflow when executed as a script. Importing the module defines its functions without training the model.

The [main guide](../README.md#dataset-and-encoding) explains each data column and mapping, and its [training section](../README.md#cell-3--clean-split-train-and-evaluate) explains the forest and metrics.

## Generated files

Each plotting function creates a figure, uses `tight_layout()`, writes a PNG at 140 DPI, and closes the figure to release plotting resources.

| File under `qt4_app/generated/` | Contents |
| --- | --- |
| `feature_importance.png` | Horizontal importance chart for all eight model features. |
| `acceptance_distribution.png` | Counts of accepted and rejected records in the cleaned dataset. |
| `age_distribution.png` | Age histograms separated by outcome. |
| `sine_wave.png` | Standalone sine-wave example. |
| `scatter_plot.png` | Standalone random scatter example with varying marker sizes and colors. |
| `summary.txt` | Row counts, model type, test MSE and R², top three features, and the script's dataset note. |

OOB scoring is enabled during training, but the exporter does not include that score in its summary. It also does not export predictions or serialize the model. Unlike the notebook, it has no Ollama or seaborn import.

## Errors and refresh behavior

The script has no exception handler. Missing input, missing packages, invalid data, or file-writing failures terminate execution and produce a traceback. In Qt, standard output and standard error appear in the process-output panel.

Outputs are written sequentially to fixed filenames, not as one atomic update. A failed run can leave a mixture of old and new files. The Qt app reloads them automatically only after a successful process exit. Python output may be buffered because the app launches `python` without `-u`, so console text may arrive in batches.
