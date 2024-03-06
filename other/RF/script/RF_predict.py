import joblib
import pandas as pd

def load_model_and_vect(model_path, vect_path):
    rf = joblib.load(model_path)
    vect = joblib.load(vect_path)
    return rf, vect

def predict_label(list6, rf, vect):
    data_l = pd.DataFrame(list6, columns=['age', 'gender', 'BMI', 'dress', 'in_car_temperature', 'RH', 'height', 'wind_speed'])
    data_dict = data_l.to_dict(orient='records')
    data_one = vect.transform(data_dict)
    result_sum = rf.predict(data_one)
    return result_sum.tolist()