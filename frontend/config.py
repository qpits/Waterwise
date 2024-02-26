from os import environ, path
import os



class Config:

    # General Flask Config
    SECRET_KEY = b'ergergergergegg/'
    USE_PROXYFIX = True

    APPLICATION_ROOT = '/'

    FLASK_APP = 'app.py'
    FLASK_RUN_HOST = '0.0.0.0'
    FLASK_RUN_PORT = 8008

    FLASK_DEBUG = 1
    FLASK_ENV = "development" #production
    #FLASK_ENV = "production"  # production

    DEBUG = True
    TESTING = False #True

    SESSION_TYPE = 'sqlalchemy' #'redis'
    SESSION_SQLALCHEMY_TABLE = 'sessions'
    SESSION_COOKIE_NAME = 'my_cookieGetFace'
    SESSION_PERMANENT = True

    CACHE_TYPE = "simple"  # Flask-Caching related configs
    CACHE_DEFAULT_TIMEOUT =  100

