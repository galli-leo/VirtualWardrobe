import numpy as np
import pandas as pd

from train import net


if __name__ == '__main__':
    df_test = pd.read_csv('examples/mnist/test.csv')
    X = df_test.values.reshape(-1, 1, 28, 28).astype(np.float32)
    X = X / 255
    net.load_params_from('./model_weights.pkl')
    y_pred = net.predict(X)
    df = pd.DataFrame({'ImageId': range(1, len(df_test) + 1), 'Label': y_pred})
    df.to_csv('examples/mnist/submission.csv', index=False)
