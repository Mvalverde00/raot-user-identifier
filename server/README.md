# Setup

### Hard dependencies
sudo apt-get install postgresql
sudo apt-get install postgresql-server-dev-all
sudo apt-get install postgresql-common

pip3 install flask
pip3 install flask-sqlalchemy
pip3 install psycopg2

### Optional
Make it so postgresql launches on startup:
sudo systemctl enable postgresql


# Running
Before running the app for the first time, run
`export FLASK_APP=hello`

flask run --host=0.0.0.0 --port=80
