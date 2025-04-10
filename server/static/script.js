function atualizarMensagens() {
    fetch('/mensagens')
        .then(response => response.json())
        .then(data => {
            let mensagensDiv = document.getElementById('mensagens');
            mensagensDiv.innerHTML = '';

            if (data.length > 0) {
                data.forEach(msg => {
                    let div = document.createElement('div');
                    div.className = 'mensagem';
                    div.textContent = msg;
                    mensagensDiv.appendChild(div);
                });

                let ultimaMensagem = data[data.length - 1];
              
            } else {
                mensagensDiv.innerHTML = '<p>Nenhuma mensagem recebida ainda.</p>';
            }
        });
}



function resetarMensagens() {
    fetch('/reset', { method: 'POST' })
        .then(() => atualizarMensagens());
}

function mostrarSecao(selecao) {
  
    document.getElementById('logs-container').style.display = 'none';
    document.getElementById(selecao).style.display = 'block';
}

setInterval(atualizarMensagens, 2000);