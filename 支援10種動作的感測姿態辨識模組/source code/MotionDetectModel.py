import os
import sktime
import numpy as np
import pickle
import argparse
from sklearn.model_selection import cross_val_score
from sklearn.model_selection import train_test_split
from sklearn.pipeline import Pipeline
from sklearn.model_selection import KFold
from sklearn.tree import DecisionTreeClassifier
from sktime.classification.compose import ColumnEnsembleClassifier
from sktime.classification.dictionary_based import BOSSEnsemble
from sktime.classification.interval_based import TimeSeriesForestClassifier
from sktime.classification.shapelet_based import MrSEQLClassifier
from sktime.classification.distance_based import KNeighborsTimeSeriesClassifier
from sktime.transformations.panel.tsfresh import TSFreshFeatureExtractor
from sktime.transformations.panel.compose import ColumnConcatenator
from sktime.transformations.panel.segment import RandomIntervalSegmenter
from sktime.utils.data_io import load_from_tsfile_to_dataframe
from sktime.classification.shapelet_based import ShapeletTransformClassifier


def get_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', type=str)
    parser.add_argument('-o', '--output', type=str)
    return parser

if __name__ == '__main__':
    
    parser = get_parser()
    args = parser.parse_args()
    
    input_data = args.input
    output_model = args.output
    
    data_x,data_y = load_from_tsfile_to_dataframe(input_data)
    X_train, X_test, y_train, y_test = train_test_split(data_x, data_y)

    clf = ColumnEnsembleClassifier(
        estimators=[
           ("TSF0", TimeSeriesForestClassifier(n_estimators=100), [0]),
           ("BOSSEnsemble3", BOSSEnsemble(max_ensemble_size=5), [3]),
        ]
    )

    #steps = [
    #    ("concatenate", ColumnConcatenator()),
    #    ("classify", TimeSeriesForestClassifier(n_estimators=100)),
    #]
    #clf = Pipeline(steps)
    
    #clf = MrSEQLClassifier()

    seed = 7
    kfold = KFold(n_splits=3, shuffle=True, random_state=seed)
    scores = cross_val_score(estimator=clf, X=data_x, y=data_y, cv=kfold)

    print(clf.fit(data_x,data_y))
    print(scores.mean())
    
    predicts = clf.predict(X_test)
    print(predicts)

    with open(output_model, 'wb') as f:
        pickle.dump(clf, f)