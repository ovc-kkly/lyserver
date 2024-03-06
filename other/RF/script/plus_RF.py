# !/usr/bin/env python
# -*- coding: UTF-8 -*-
"""
@Project ：decision-making tree 
@File ：randomforest.py
@Author ：LY
@Date ：2022/7/25 14:54 
"""
import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.feature_extraction import DictVectorizer
from sklearn.model_selection import cross_val_score, GridSearchCV  # 导入网格搜索（用于参数调优）
from sklearn.metrics import accuracy_score, precision_score, f1_score, recall_score
from sklearn.multioutput import MultiOutputClassifier
from sklearn.pipeline import Pipeline
import joblib
import tqdm


def get_data(datacsv):
    """
    加载数据集
    :return:
    """
    # filepath = r"D:\研究生\研究生其他\数据集\data_csv\dataset_pmv_need_temp.csv"
    # filepath1 = r"data_/dataset_pmv_need_temp.csv"
    data = pd.read_csv(datacsv, encoding='gbk')
    return data


def mul_target_encoder(data):
    """
    把多个目标变成一个大目标
    :param data:
    :return:
    """
    data_target = data[['need_temperature', 'The_wind_type', 'wind_scale', 'direction']].astype(str)

    data_target1 = []
    da_shape = data_target.shape
    for i in range(da_shape[0]):
        temp1 = data_target.iloc[i, 0]
        temp2 = data_target.iloc[i, 1]
        temp3 = data_target.iloc[i, 2]
        temp4 = data_target.iloc[i, 3]
        # temp5 = data_target.iloc[i, 4]
        data_target1.append(temp1 + "," + temp2 + "," + temp3 + "," + temp4)
    return data_target1


def data_deal(data):
    """
    数据预处理，选取需要的特征，标签。分出后面一点数据来做预测
    :param data:
    :return:
    """
    data_features = data[['age', 'gender', 'BMI', 'dress', 'in_car_temperature', 'height', 'RH', 'wind_speed']]  # 选取需要的特征
    data_targets = data[['need_temperature', 'The_wind_type', 'wind_scale', 'direction']]

    data_predict_features = data_features[-50:]  # 输入预测函数的特征值
    data_predict_targets = data_targets[-50:]  # 验证预测结果的目标值

    # 将建模数据删除最后50行
    data_features = data_features[:-50]  # 建模所需的特征值x
    data_targets = data_targets[:-50]  # 建模所需的目标值y
    return data_features, data_targets, data_predict_features, data_predict_targets


def feature_extraction(data_features, data_predict_features):
    """
    字典特征提取
    将任意数据（如文本或图像）转换为可用于机器学习的数字特征
    把特征(文字、数字)转为数字特征
    :param data_features:
    :param data_predict_features:
    :return:
    """
    vect = DictVectorizer(sparse=False)  # 实例化一个字典特征分类器
    # data_features 字典特征提取
    data_features1 = data_features.to_dict(orient='records')  # 把data_features转换成字典
    data_features2 = vect.fit_transform(data_features1)  # 对字典进行特征提取，数字不变，(男女)->(0,1)one hot 编码

    # data_predict_features 字典特征提取
    data_predict_features1 = data_predict_features.to_dict(orient='records')
    data_predict_features2 = vect.fit_transform(data_predict_features1)

    return data_features2, data_predict_features2, vect


def check_model_xinneng(grid_search, x_text, y_test):
    """
    计算一下准确率等
    :param grid_search:
    :param x_text:
    :param y_test:
    :return:
    """
    print("基于训练集的最佳参数组合：", grid_search.best_params_)
    print("基于训练集的最佳准确率：", grid_search.best_score_)
    y_pred = grid_search.predict(x_text)
    accuracy = grid_search.score(x_text, y_test)

    # 计算准确率
    print("基于测试集的平均准确率：", accuracy)

    # 计算每个标签的召回率
    # 转置二维列表，将每一列作为一个独立的列表
    column_lists = [list(column) for column in zip(*y_pred)]
    # 将每个特征拆分为独立的列表
    feature_lists = [y_test[column].tolist() for column in y_test.columns]
    # print("基于测试集的平均召回率：", recall)
    # 计算准确率
    accuracy = [0, 1, 2, 3]
    for i in range(4):
        accuracy[i] = accuracy_score(feature_lists[i], column_lists[i])
    print("基于测试集的准确率：", accuracy)

    # 计算查准率
    precision_micro = [0, 1, 2, 3]
    precision_macro = [0, 1, 2, 3]
    for i in range(4):
        precision_micro[i] = precision_score(feature_lists[i], column_lists[i], average='micro')
        precision_macro[i] = precision_score(feature_lists[i], column_lists[i], average='macro')
    print("基于测试集的微观查准率：", precision_micro)
    print("基于测试集的宏观查准率：", precision_macro)
    # 计算查全率
    recall_micro = [0, 1, 2, 3]
    recall_macro = [0, 1, 2, 3]
    for i in range(4):
        recall_micro[i] = recall_score(feature_lists[i], column_lists[i], average='micro')
        recall_macro[i] = recall_score(feature_lists[i], column_lists[i], average='macro')
    print("基于测试集的微观查全率：", recall_micro)
    print("基于测试集的宏观查全率：", recall_macro)

    # 计算F1系数
    f1_micro = [0, 1, 2, 3]
    f1_macro = [0, 1, 2, 3]
    for i in range(4):
        f1_micro[i] = f1_score(feature_lists[i], column_lists[i], average='micro')
        f1_macro[i] = f1_score(feature_lists[i], column_lists[i], average='macro')
    print("基于测试集的微观F1系数：", f1_micro)
    print("基于测试集的宏观F1系数：", f1_macro)


def split_data(data_features, data_targets):
    """
    划分并且训练
    :param data_features:
    :param data_targets:
    :return:
    """
    # 传入建模所需的特征值数据和目标值数据进行划分训练集(0.75)和测试集(0.25)

    x_train, x_test, y_train, y_test = train_test_split(data_features, data_targets, test_size=0.10)
    scores = []
    uni, counts = np.unique(y_train, return_counts=True)
    cv_ = 2
    min_count = min(counts)
    if min_count == 1:
        cv_ = 2
    elif min_count <= 10:
        cv_ = min_count
    elif min_count > 10:
        cv_ = 10
    # 交叉验证调参-min_samples_leaf

    param_grid = {'multi_rf__estimator__n_estimators': [199, 200, 201],
                  'multi_rf__estimator__max_depth': [24],
                  'multi_rf__estimator__max_features': [8],
                  'multi_rf__estimator__min_samples_leaf': [1],
                  'multi_rf__estimator__min_samples_split': [2]}
    rfc = RandomForestClassifier(random_state=42, n_jobs=-1)
    multi_rf = MultiOutputClassifier(rfc)
    # 创建Pipeline对象，将MultiOutputClassifier放入Pipeline中
    pipeline = Pipeline([('multi_rf', multi_rf)])

    # 创建GridSearchCV对象
    GS1 = GridSearchCV(pipeline, param_grid, cv=cv_)
    GS1.fit(x_train, y_train)
    check_model_xinneng(GS1, x_test, y_test)
    return GS1


def rf_predict(GS1, data_predict_features):
    # 预测
    predict_result = GS1.predict(data_predict_features)
    print("预测结果:", predict_result)
    return predict_result


def save_model(rf, vect, path_rf, path_vect):
    try:
        # 尝试保存模型和向量化器
        joblib.dump(rf, path_rf)
        joblib.dump(vect, path_vect)
        # 如果没有异常发生，保存成功
        return True
    except Exception as e:
        # 如果发生异常，保存失败
        print(f"保存模型时出错: {e}")
        return False


def main_fun(data_csv):
    # data = get_data()
    data_features, data_targets, data_predict_features, data_predict_targets = tqdm(data_deal(data_csv), desc="Data Processing")  # 提取数据
    data_featured, data_predict_featured, vect = tqdm(feature_extraction(data_features, data_predict_features), desc="Feature Extraction")
    # rf, accuracy, param_grid= split_data(data_featured, data_targets)  # 划分数据集并且训练
    rf = tqdm(split_data(data_featured, data_targets), desc="Model Training")  # 划分数据集并且训练

    # 预测，输入预测所需的特征值
    result = rf.predict(data_predict_featured)
    # draw_img(rf, param_grid)
    save_model(rf, vect)

    final_result = result

    print("预测结果:", final_result)


if __name__ == '__main__':
    main_fun()


# def check_model_xinneng(grid_search, x_text, y_test):
#     """
#     计算一下准确率等
#     :param grid_search:
#     :param x_text:
#     :param y_test:
#     :return:
#     """
#     print("基于训练集的最佳参数组合：", grid_search.best_params_)
#     print("基于训练集的最佳准确率：", grid_search.best_score_)
#     y_pred = grid_search.predict(x_text)
#     accuracy = grid_search.score(x_text, y_test)
#
#     # 计算准确率
#     print("基于测试集的平均准确率：", accuracy)
#
#     # 计算每个标签的召回率
#     # 转置二维列表，将y_pred的每一列作为一个独立的列表
#     column_lists = [list(column) for column in zip(*y_pred)]
#     # 将y_test拆分为独立的列表
#     feature_lists = [y_test[column].tolist() for column in y_test.columns]
#     # print("基于测试集的平均召回率：", recall)
#     # 计算准确率
#     # accuracy = [0, 1, 2, 3]
#     # for i in range(4):
#     #     accuracy[i] = accuracy_score(feature_lists[i], column_lists[i])
#     # print("基于测试集的准确率：", accuracy)
#
#     # 计算查准率
#     precision_micro = [0, 1, 2, 3]
#     precision_macro = [0, 1, 2, 3]
#     for i in range(4):
#         precision_micro[i] = precision_score(feature_lists[i], column_lists[i], average='micro')
#         precision_macro[i] = precision_score(feature_lists[i], column_lists[i], average='macro')
#     print("基于测试集的微观查准率：", precision_micro)
#     print("基于测试集的宏观查准率：", precision_macro)
#     # 计算查全率
#     recall_micro = [0, 1, 2, 3]
#     recall_macro = [0, 1, 2, 3]
#     for i in range(4):
#         recall_micro[i] = recall_score(feature_lists[i], column_lists[i], average='micro')
#         recall_macro[i] = recall_score(feature_lists[i], column_lists[i], average='macro')
#     print("基于测试集的微观查全率：", recall_micro)
#     print("基于测试集的宏观查全率：", recall_macro)
#
#     # 计算F1系数
#     f1_micro = [0, 1, 2, 3]
#     f1_macro = [0, 1, 2, 3]
#     for i in range(4):
#         f1_micro[i] = f1_score(feature_lists[i], column_lists[i], average='micro')
#         f1_macro[i] = f1_score(feature_lists[i], column_lists[i], average='macro')
#     print("基于测试集的微观F1系数：", f1_micro)
#     print("基于测试集的宏观F1系数：", f1_macro)