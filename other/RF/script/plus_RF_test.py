#!/usr/bin/env python
# -*- coding: UTF-8 -*-
"""
@Project ：decision-making tree 
@File ：randomforest_test.py
@Author ：LY
@Date ：2022/9/8 17:10
"""
import pandas as pd
import joblib
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, precision_score, f1_score, recall_score
from sklearn.ensemble import RandomForestClassifier
from numpy import argmax
from sklearn.preprocessing import LabelEncoder
from sklearn.preprocessing import OneHotEncoder
from sklearn.feature_extraction import DictVectorizer
import datetime
import plus_RF


def get_input():
    """
    这个是获取输入的函数，在这边改输入就行
    :return:
    """
    #            年龄，  性别，   BMI,   衣着   ，    车内温度，        身高，  湿度，         风速
    #           'age','gender','BMI','dress','in-car temperature','RH','height(m)', 'wind_speed'
    in_list = [['青年', '女', 17.7, '春秋装', 24.02, 56.12, 1.53, 0.5]]
    # in_list = [['青年', '女', 23.01, '春秋装', 16.36, 61, 1.6454]]
    # in_list = [['青年', '女', 17.67, '春秋装', 17.64, 59, 1.533]]
    # air_temperature = [18.21, 19.01, 20.0, 21.3, 22.45, 23.55]
    # relative_humidity = [65, 64.3, 62.4, 61.2, 60.7, 59.6]  # 相对湿度（百分比）
    return in_list


def predict_label(list6, rf, vect):
    """

    :param list6:
    :return:
    """
    # data_features = data[['age', 'gender', 'BMI', 'dress', 'in-car temperature', 'height(m)', 'RH']]
    data_l = pd.DataFrame(list6, columns=['age', 'gender', 'BMI', 'dress', 'in_car_temperature', 'height', 'RH', 'wind_speed'])
    data_dict = data_l.to_dict(orient='records')  # 把data_features转换成字典
    data_one = vect.transform(data_dict)  # 对字典进行特征提取，数字不变，(男女)->(0,1)one hot 编码

    result_sum = rf.predict(data_one)       # 预测结果为numpy数组

    return result_sum


def main():
    """

    :return:
    """

    all_rf = joblib.load(r"X:\Battery thermal management\model\all_model.pkl")

    all_vect = joblib.load(r"X:\Battery thermal management\model\all_vect.pkl")

    in_list = get_input()  # 获取输入数据
    result0 = predict_label(in_list, all_rf, all_vect)

    print(result0)  # 输出预测结果
    # decision(list(chain.from_iterable(result_2)))


if __name__ == '__main__':
    main()
    aa = datetime.datetime.now()  # 开始时间点


    bb = datetime.datetime.now()  # 结束时间点

    cc = bb - aa  # 运行时间，单位是  时:分:秒
    print('运行时间是：', cc)
