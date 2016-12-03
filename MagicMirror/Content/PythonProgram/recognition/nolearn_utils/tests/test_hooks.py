import pytest
from mock import MagicMock


@pytest.fixture()
def mock_train_history():
    def make_train_history(valid_loss, train_loss, epoch):
        keys = ['valid_loss', 'train_loss', 'epoch']
        values = zip(valid_loss, train_loss, epoch)
        return [{k: v for k, v in zip(keys, value)} for value in values]
    return make_train_history


def test_early_stopping(mock_train_history):
    from nolearn_utils.hooks import EarlyStopping
    early_stopper = EarlyStopping(patience=3)
    train_history = mock_train_history(valid_loss=[0.7, 0.6, 0.6, 0.6, 0.6],
                                       train_loss=[0.3, 0.3, 0.3, 0.3, 0.3],
                                       epoch=[1, 2, 3, 4, 5])
    net = MagicMock()
    net.get_all_param = MagicMock()
    early_stopper(net, train_history[:1])
    early_stopper(net, train_history[:2])
    early_stopper(net, train_history[:3])
    early_stopper(net, train_history[:4])
    with pytest.raises(StopIteration):
        early_stopper(net, train_history[:5])
