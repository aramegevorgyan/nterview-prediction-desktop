from __future__ import print_function

import os

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_squared_error, r2_score
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder


ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DATASET_PATH = os.path.join(ROOT_DIR, "logatta.csv")
OUTPUT_DIR = os.path.join(ROOT_DIR, "qt4_app", "generated")


def ensure_output_dir():
    if not os.path.isdir(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)


def load_dataframe():
    df = pd.read_csv(DATASET_PATH)
    target_col = "accepted for the interview"

    df["BusinessTravel"] = df["BusinessTravel"].map(
        {"Non-Travel": 0, "Travel_Rarely": 1, "Travel_Frequently": 2}
    )
    df["MaritalStatus"] = df["MaritalStatus"].map(
        {"Single": 0, "Married": 1, "Divorced": 2}
    )
    df["OverTime"] = df["OverTime"].map({"No": 0, "Yes": 1})
    df["Gender"] = df["Gender"].map({"Female": 0, "Male": 1})
    df[target_col] = (
        df[target_col].astype(str).str.upper().map({"FALSE": 0, "TRUE": 1})
    )

    df = df.dropna().copy()
    encoder = LabelEncoder()
    for column in df.select_dtypes(include=["object"]).columns:
        df[column] = encoder.fit_transform(df[column])

    return df, target_col


def train_model(df, target_col):
    feature_cols = [column for column in df.columns if column != target_col]
    x_values = df[feature_cols].values
    y_values = df[target_col].values

    x_train, x_test, y_train, y_test = train_test_split(
        x_values, y_values, test_size=0.2, random_state=42
    )

    model = RandomForestRegressor(
        n_estimators=100, random_state=42, oob_score=True
    )
    model.fit(x_train, y_train)

    y_pred = model.predict(x_test)
    mse = mean_squared_error(y_test, y_pred)
    r2_value = r2_score(y_test, y_pred)

    return model, feature_cols, mse, r2_value


def save_feature_importance_plot(model, feature_cols):
    importances = model.feature_importances_
    feature_series = pd.Series(importances, index=feature_cols).sort_values(
        ascending=True
    )

    plt.figure(figsize=(8, 5))
    feature_series.plot(kind="barh", color="steelblue", edgecolor="white")
    plt.title("Feature Importances (RandomForest)")
    plt.xlabel("Importance Score")
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "feature_importance.png"), dpi=140)
    plt.close()

    return feature_series.sort_values(ascending=False).head(3)


def save_distribution_plot(df, target_col):
    plt.figure(figsize=(5, 3))
    df[target_col].value_counts().rename({0: "Rejected", 1: "Accepted"}).plot(
        kind="bar", color=["#e05c5c", "#5c9be0"], edgecolor="white"
    )
    plt.title("Interview Acceptance Distribution")
    plt.xlabel("")
    plt.ylabel("Count")
    plt.xticks(rotation=0)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "acceptance_distribution.png"), dpi=140)
    plt.close()


def save_age_histogram(df, target_col):
    plt.figure(figsize=(7, 4))
    for label, group in df.groupby(target_col):
        plt.hist(
            group["Age"],
            bins=15,
            alpha=0.6,
            label="Accepted" if label else "Rejected",
        )

    plt.title("Age Distribution by Interview Result")
    plt.xlabel("Age")
    plt.ylabel("Count")
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "age_distribution.png"), dpi=140)
    plt.close()


def save_line_plot():
    x_values = np.linspace(0, 10, 100)
    y_values = np.sin(x_values)

    plt.figure(figsize=(8, 4))
    plt.plot(x_values, y_values, label="sin(x)", color="blue", linestyle="--")
    plt.title("Simple Sine Wave")
    plt.xlabel("X-axis")
    plt.ylabel("Y-axis")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "sine_wave.png"), dpi=140)
    plt.close()


def save_scatter_plot():
    np.random.seed(42)
    data = {
        "x_values": np.random.rand(50) * 10,
        "y_values": np.random.rand(50) * 10 + np.random.randn(50) * 2,
        "sizes": np.random.rand(50) * 500,
        "colors": np.random.rand(50),
    }
    df = pd.DataFrame(data)

    plt.figure(figsize=(9, 5))
    plt.scatter(
        df["x_values"],
        df["y_values"],
        s=df["sizes"],
        c=df["colors"],
        cmap="viridis",
        alpha=0.7,
        label="Random Data Points",
    )
    plt.colorbar(label="Color Intensity")
    plt.title("Scatter Plot with Varying Size and Color")
    plt.xlabel("Feature X")
    plt.ylabel("Feature Y")
    plt.legend()
    plt.grid(True, linestyle=":", alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "scatter_plot.png"), dpi=140)
    plt.close()


def save_summary(df, mse, r2_value, top_features):
    target_col = "accepted for the interview"
    accepted_count = int((df[target_col] == 1).sum())
    rejected_count = int((df[target_col] == 0).sum())

    lines = [
        "Project summary",
        "---------------",
        "Dataset rows: %d" % len(df),
        "Accepted rows: %d" % accepted_count,
        "Rejected rows: %d" % rejected_count,
        "Model type: RandomForestRegressor",
        "Mean Squared Error: %.4f" % mse,
        "R-squared: %.4f" % r2_value,
        "",
        "Top features:",
    ]

    for feature_name, score in top_features.items():
        lines.append("- %s: %.4f" % (feature_name, score))

    lines.append("")
    lines.append(
        "Note: the repository currently predicts interview acceptance, not heart disease."
    )

    with open(os.path.join(OUTPUT_DIR, "summary.txt"), "w") as handle:
        handle.write("\n".join(lines))

    return lines


def main():
    ensure_output_dir()
    dataframe, target_col = load_dataframe()
    model, feature_cols, mse, r2_value = train_model(dataframe, target_col)
    top_features = save_feature_importance_plot(model, feature_cols)
    save_distribution_plot(dataframe, target_col)
    save_age_histogram(dataframe, target_col)
    save_line_plot()
    save_scatter_plot()
    summary_lines = save_summary(dataframe, mse, r2_value, top_features)
    print("Python model run completed.")
    print("Dataset:", DATASET_PATH)
    print("Output directory:", OUTPUT_DIR)
    print("")
    for line in summary_lines:
        print(line)
    print("")
    print("Plots and summary written to %s" % OUTPUT_DIR)


if __name__ == "__main__":
    main()
