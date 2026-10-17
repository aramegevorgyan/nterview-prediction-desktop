# Interview Acceptance Prediction & Qt Viewer

A Python analysis workflow and C++/Qt desktop viewer built around `logatta.csv`. The Python code encodes candidate data, trains a 100-tree random forest regressor on binary interview outcomes, evaluates numeric predictions, and creates charts. The desktop app runs the export script and displays its generated images and summary.

## How the parts connect

```text
logatta.csv ──→ project2_1.ipynb ──→ inline metrics and charts
     │
     └──→ scripts/generate_plots.py ──→ qt4_app/generated/
                    ↑                          │
             Qt starts Python                 ↓
                    └────────────── Qt displays summary and PNGs
```

The notebook and export script implement separate versions of the analysis. The script does not execute the notebook. Both use `RandomForestRegressor`, not a classifier. Ollama is imported in the notebook but never called, so no AI-generated explanation is produced.

## File guide

| File or directory | Role |
| --- | --- |
| [project2_1.ipynb](project2_1.ipynb) | Seven notebook cells for loading, preprocessing, training, evaluation, and charts. |
| [logatta.csv](logatta.csv) | Input table: 1,492 records, eight predictor columns, and one target column. |
| [scripts/generate_plots.py](scripts/generate_plots.py) | Standalone analysis that exports five PNGs and a text summary. |
| [scripts/README.md](scripts/README.md) | Explanation of every exporter function and output. |
| [qt4_app/main.cpp](qt4_app/main.cpp) | Creates the Qt application and starts its event loop. |
| [qt4_app/mainwindow.h](qt4_app/mainwindow.h) | Declares the window, widgets, process callbacks, and plot registry. |
| [qt4_app/mainwindow.cpp](qt4_app/mainwindow.cpp) | Builds the interface, launches Python, captures console output, and refreshes results. |
| [qt4_app/CMakeLists.txt](qt4_app/CMakeLists.txt) | CMake configuration for the Qt Widgets executable. |
| [qt4_app/qt4_app.pro](qt4_app/qt4_app.pro) | Alternative qmake build configuration. |
| [qt4_app/README.md](qt4_app/README.md) | C++ function guide, build instructions, runtime paths, and existing issues. |
| `qt4_app/generated/` | Exported summary and chart files, explained in the script guide. |
| `qt4_app/build/` | Existing machine-specific CMake configuration files, explained in the Qt guide. |
| `README.md` | This setup, dataset, and notebook guide. |

## Run the Python workflows

From the repository root:

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install pandas numpy scikit-learn matplotlib
python scripts/generate_plots.py
```

On Windows, activate with `.venv\Scripts\activate` in Command Prompt or `.venv\Scripts\Activate.ps1` in PowerShell.

The script writes to `qt4_app/generated/`, overwriting files with the same names. It resolves the dataset relative to its own source file, so it also works when launched from another working directory. It does not require Qt, Jupyter, seaborn, or Ollama.

For the notebook, install its additional imports and a notebook environment:

```bash
python -m pip install jupyterlab ipykernel seaborn ollama
python -m ipykernel install --user --name uniproject --display-name "UniProject"
python -m jupyter lab
```

Open `project2_1.ipynb`, select the **UniProject** kernel, and run cells in order. Its working directory must be the repository root because it opens `Path('logatta.csv')`. The Ollama Python package is required by the current import, but no Ollama server or downloaded language model is used.

The notebook metadata records Python 3.14.4. Dependencies are not pinned, and the repository does not define a tested version range. For the desktop application, see the [Qt build guide](qt4_app/README.md#build-configuration).

## Dataset and encoding

The CSV has no empty fields. Both Python workflows apply the same explicit mappings:

| Column | Meaning or values | Handling |
| --- | --- | --- |
| `Age` | Numeric age field | Used unchanged. |
| `BusinessTravel` | Travel category | `Non-Travel → 0`, `Travel_Rarely → 1`, `Travel_Frequently → 2`. |
| `Education` | Numeric codes 1–5 | Used unchanged; category labels are not supplied. |
| `MaritalStatus` | Marital status | `Single → 0`, `Married → 1`, `Divorced → 2`. |
| `OverTime` | Overtime category | `No → 0`, `Yes → 1`. |
| `EmployeeNumber` | Employee identifier | Included as a feature, not removed. |
| `DailyRate` | Numeric rate field | Used unchanged; units are not documented. |
| `Gender` | Gender category | `Female → 0`, `Male → 1`. |
| `accepted for the interview` | Target outcome | Converted to uppercase strings, then `FALSE → 0`, `TRUE → 1`. |

The target contains **157 accepted** and **1,335 rejected** rows. Its class imbalance matters when interpreting results. These are the labels stored in the file; the repository does not include a separate explanation of how they were collected or assigned.

`map(...)` converts matching categories to numbers and turns unknown categories into missing values. `dropna()` removes rows with a missing value in any field. The target's `.astype(str).str.upper()` supports both string and Boolean values inferred by pandas; it does not strip whitespace.

A `LabelEncoder` loop then encodes any remaining columns selected with `include=['object']`. It refits the same encoder separately per column and does not save mappings. The supplied categorical features already have explicit mappings, so this fallback is normally unnecessary. It only processes the dtypes selected by that expression, not every possible pandas string dtype.

## Notebook walkthrough

### Cell 1 — Dependency installation

```python
%pip install pandas numpy scikit-learn matplotlib ollama
```

`%pip` installs into the notebook kernel's environment. This cell omits `seaborn`, although cell 2 imports it; the setup above includes it.

### Cell 2 — Imports, loading, and category mapping

- `Path` builds and checks the CSV path; a missing file raises `FileNotFoundError` with the resolved location.
- `sys.executable` identifies the active Python interpreter.
- pandas reads and transforms the table; NumPy supplies arrays and random data; Matplotlib draws charts.
- `train_test_split`, `LabelEncoder`, `RandomForestRegressor`, `mean_squared_error`, and `r2_score` support the analysis.
- `ollama`, `seaborn`, `LogisticRegression`, and `accuracy_score` are imported but unused.

`pd.read_csv(csv_path)` loads `df`; `df.shape` prints `(1492, 9)`, and `df.info()` prints column types and non-null counts. The rest of the cell defines `target_col` and applies the mappings in the dataset table.

### Cell 3 — Clean, split, train, and evaluate

After `dropna()` and the fallback encoder loop, `feature_cols` collects every column except the target. `df[feature_cols].values` creates the feature array `X`, and `df[target_col].values` creates the label array `y`.

```python
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)
regressor = RandomForestRegressor(
    n_estimators=100, random_state=42, oob_score=True
)
regressor.fit(X_train, y_train)
y_pred = regressor.predict(X_test)
```

With all supplied rows retained, the split produces 1,193 training rows and 299 test rows. `random_state=42` fixes the split and forest randomness for the same inputs and environment. The split is not stratified.

Each tree learns a regression prediction from a bootstrap sample of training rows; the forest averages the trees' predictions. Because the target is 0 or 1, these predictions are numeric acceptance scores between 0 and 1. The code neither thresholds them into class labels nor calibrates them as probabilities.

| Metric | What the code measures |
| --- | --- |
| Out-of-bag score | The regressor's R² estimate using predictions from trees whose bootstrap samples excluded each training row. It is not accuracy. |
| Mean squared error (MSE) | Average squared difference between the test target values and predicted scores; lower is better. |
| R-squared (R²) | Test fit relative to predicting the test-target mean; 1 is perfect, 0 matches that reference, and negative values are possible. |

The cell prints rounded metrics. It does not compute precision, recall, F1, a confusion matrix, or classification accuracy. No model file is saved.

### Cell 4 — Section heading

The Markdown cell reads `Feature Importance & LLM Analysis`. It is a heading only; there is no LLM analysis cell following it.

### Cell 5 — Feature importance

`regressor.feature_importances_` supplies one importance value per feature. A pandas Series pairs these with `feature_cols`, sorts them ascending, and draws a horizontal bar chart. `tight_layout()` fits labels inside the figure, and `show()` displays it.

The same Series is sorted descending and reduced with `.head(3)` to print the three highest values. These are the forest's impurity-based importance scores, not regression coefficients, causal effects, or the direction of a feature's influence.

### Cell 6 — Dataset charts

`df_plot = df.copy()` preserves a separate table for plotting. `value_counts()` counts target labels, and `rename({0: 'Rejected', 1: 'Accepted'})` gives the bar chart readable labels.

The second chart groups rows by target and overlays a 15-bin age histogram for each group. `alpha=0.6` makes overlapping bars visible, and a legend identifies the groups.

### Cell 7 — Synthetic plotting examples

The sine chart uses `np.linspace(0, 10, 100)` for 100 x coordinates and `np.sin(x)` for y coordinates. It draws a blue dashed line with labels, a legend, and a grid.

The scatter chart seeds NumPy with `42` and generates 50 points. Horizontal coordinates are uniform random values times 10; vertical coordinates add normal noise times 2 to uniform values times 10. Separate random arrays control marker area (`s=`, up to 500) and color (`c=`, using `viridis`). A color bar explains the color scale; `alpha=0.7` controls transparency.

These two charts do not use candidate data or predictions. This cell overwrites `df` and `y` with plotting data, so restart and rerun from the loading cell before repeating training or dataset analysis.

## Saved results

The committed notebook outputs contain OOB R² **0.8469**, test MSE **0.0243**, and test R² **0.7384**. The committed `qt4_app/generated/summary.txt` also records MSE **0.0243** and R² **0.7384**, with these top features:

| Feature | Saved importance |
| --- | --- |
| `MaritalStatus` | 0.3018 |
| `OverTime` | 0.2944 |
| `Education` | 0.1218 |

These are saved artifacts, not newly verified benchmark results. Rerun the analysis to obtain metrics for your environment. The exporter and notebook train independently, and the viewer displays whichever files are currently in `generated/`.

## Implementation limits

There is no prediction form, API, persisted model, or saved preprocessing pipeline. Every exporter run trains from scratch. Identifier and categorical-code features are passed directly to the forest. Evaluation uses one train/test split, with no hyperparameter search or cross-validation.

The Qt source has existing header, signal-connection, and executable-location issues documented in its [implementation notes](qt4_app/README.md#existing-implementation-issues). This documentation describes the code as supplied; it does not change its behavior.
