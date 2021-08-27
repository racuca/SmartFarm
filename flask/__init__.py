import os
from datetime import datetime
from flask import Flask, render_template, request


sensor_list = []

def create_app(test_config=None):
    # create and configure the app
    app = Flask(__name__, instance_relative_config=True)
    app.config.from_mapping(
        SECRET_KEY='dev',
        DATABASE=os.path.join(app.instance_path, 'flaskr.sqlite'),
    )

    if test_config is None:
        # load the instance config, if it exists, when not testing
        app.config.from_pyfile('config.py', silent=True)
    else:
        # load the test config if passed in
        app.config.from_mapping(test_config)

    # ensure the instance folder exists
    try:
        os.makedirs(app.instance_path)
    except OSError:
        pass


    @app.route('/')
    def hello():
        global sensor_list
        # return 'Hello, World!'
        print(sensor_list)
        return render_template('smartfarm.html', sensor_list=sensor_list)

    @app.route('/', methods=['POST', 'GET'])
    def updatesensor():
        global sensor_list
        jsondata = request.get_json()
        jsondata['create_date'] = datetime.now()
        sensor_list.append(jsondata)
        print(sensor_list)
        return "OK"

    return app
