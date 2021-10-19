from flask import Flask, request, jsonify, make_response, render_template, jsonify
from flask_sqlalchemy import SQLAlchemy
import re

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'postgresql:///raot_names'
db = SQLAlchemy(app)

username_matcher = re.compile(".*_[0-9][0-9]")

class UserEntry(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    # The username the player was using.
    username = db.Column(db.String(32), nullable=False)
    # The player's unchangeable UUID
    uuid = db.Column(db.String(32), nullable=False)
    # The duration in seconds the player had the username for.
    duration = db.Column(db.Integer, nullable=False)

    def __repr__(self):
        return self.username + ", " + self.uuid + ", " + str(self.duration)

@app.route("/")
def landing_page():
    return render_template("search.html")

@app.route("/submit_data", methods=['POST'])
def submit_data():
    data = request.form

    username = data.get('username', '')
    uuid = data.get('uuid', '')
    duration = int(data.get('connection_duration', -1))

    print(username, len(username))

    if len(username) <= 3:
        return make_response("Error: Username too short", 400)
    if len(uuid) != 32:
        return make_response("Error: Invalid uuid", 400)
    if duration < 0:
        return make_response("Error: Invalid connection duration", 400)

    # In the game, players cannot have the same name.  To enforce this, if two
    # or more players join the same lobby with the same name, the names will be
    # numbered, e.g. 'name', 'name_01', 'name_02', ...  For our purposes this is
    # irrelevant, so identify names which have been modified with this pattern
    # and remove the numbering, recovering the base name.
    if username_matcher.fullmatch(username):
        username = username[:-3]

    # Add data to to the database with the follow rule: if the uuid/player combo
    # already exists in the database, we add the connection_duration to the
    # current value.  Otherwise, we create a new record with
    # a value set to connection_duration.
    entry = UserEntry.query.filter_by(username=username, uuid=uuid).first()
    if entry:
        entry.duration = entry.duration + duration
    else:
        entry = UserEntry(username=username, uuid=uuid, duration=duration)
        db.session.add(entry)

    db.session.commit()

    return "Submitted player %s, %s, %d".format(entry.username, entry.uuid, entry.duration)

@app.route("/user/<uuid>")
def get_names(uuid):
    if (len(uuid)) != 32:
        return "ERROR: Invalid UUID provided."

    entries = UserEntry.query.filter_by(uuid=uuid).order_by(UserEntry.duration).all()
    strs = [str(user) for user in entries]
    return jsonify(strs)

# For debugging
@app.route("/show_all_users")
def get_all_names():
    users = UserEntry.query.order_by(UserEntry.uuid).all()
    strs = [str(user) for user in users]
    return "<br/>".join(strs)
