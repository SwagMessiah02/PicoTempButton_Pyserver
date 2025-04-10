import os
from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

mensagens = []

@app.route("/mensagem", methods=["GET"])
def receber_mensagem():
    mensagem = request.args.get("msg")
    if mensagem:
        mensagens.append(mensagem)
        print(f"Mensagem recebida: {mensagem}")
    return "Mensagem recebida"

@app.route("/mensagens", methods=["GET"])
def listar_mensagens():
    return jsonify(mensagens)

@app.route("/reset", methods=["POST"])
def resetar_mensagens():
    global mensagens
    mensagens = []
    print("Mensagens resetadas.")
    return "Mensagens resetadas"

@app.route("/", methods=["GET"])
def pagina_inicial():
    return render_template("index.html")

PORT = int(os.environ.get("PORT", 5000))
app.run(host="0.0.0.0", port=PORT)