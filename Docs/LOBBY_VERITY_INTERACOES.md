# Lobby — Verity: proximidade, interações e cutscenes

**Status:** especificação planejada; ainda não implementada.  
**Regra de integração:** NÃO adicionar esta funcionalidade ao aplicativo enquanto o lobby não estiver implementado, compilável e validado. Primeiro inspecionar a arquitetura e os componentes reais do lobby; depois integrar em módulos pequenos, sem substituir telas existentes.

## Objetivo
Permitir que o jogador se aproxime de Verity no lobby, permaneça ao lado dela e acione interações contextuais com animações e pequenas cutscenes.

## Requisitos funcionais
1. **Presença no lobby:** Verity deve aparecer como personagem separado, posicionada ao lado do ponto inicial do jogador.
2. **Proximidade:** detectar distância entre jogador e Verity; exibir o botão de interação somente dentro do alcance definido.
3. **Movimentação:** permitir que o jogador caminhe até ela usando os controles do lobby, sem teletransporte involuntário.
4. **Interações contextuais:** oferecer opções adequadas ao contexto, como conversar, acenar, abraço e beijo na bochecha. Não implementar beijo na boca nesta especificação.
5. **Consentimento e estado:** Verity deve aceitar, recusar ou encerrar cada interação de forma explícita e coerente com o estado atual; impedir sobreposição de animações.
6. **Animação:** criar estados separados para idle, olhar/virar para o jogador, caminhar, aproximação, abraço, gesto de carinho e retorno ao idle. Sincronizar transições com a cutscene.
7. **Cutscene curta:** câmera e personagens devem enquadrar a interação; permitir pular a cena; devolver o controle ao jogador ao terminar.
8. **Áudio e expressão:** expressões faciais, movimento de cabeça e falas/áudio opcionais devem acompanhar os eventos sem bloquear o jogo.
9. **Android:** manter baixo consumo de memória e processamento; carregar animações sob demanda e evitar dependências pesadas.

## Arquitetura sugerida
- LobbyInteractionController: controla alcance, seleção e estado da interação.
- VerityCompanion: estado, posicionamento e resposta da personagem.
- InteractionAnimationController: toca e sincroniza animações.
- LobbyCutsceneController: câmera, duração, skip e devolução do controle.

Os nomes acima são propostas; adaptar aos nomes e padrões existentes depois de inspecionar o código real.

## Máquina de estados mínima
IDLE -> PLAYER_NEAR -> INTERACTION_MENU -> PLAYING_INTERACTION -> RETURN_TO_IDLE

A qualquer momento permitido, o jogador pode cancelar ou pular a cutscene. Ao sair do alcance antes de iniciar, fechar o menu. Não permitir iniciar outra interação enquanto uma estiver em execução.

## Critérios de aceitação
- O lobby abre normalmente sem a funcionalidade ativada.
- Jogador e Verity aparecem em posições estáveis.
- O botão surge apenas no alcance correto.
- Cada opção toca a animação correspondente e termina no estado correto.
- Cutscene pode ser pulada e devolve o controle.
- Nenhuma animação fica travada ou sobreposta.
- Não há erros de compilação, JNI ou renderização.
- Testar em Android antes de marcar como concluído.

## Ordem de implementação
1. Inspecionar documentação e código do lobby existente.
2. Confirmar sistema de personagem, animações, câmera e input já disponíveis.
3. Integrar posicionamento e detecção de proximidade.
4. Adicionar menu e máquina de estados.
5. Integrar animações e cutscene.
6. Compilar, testar no Android e corrigir antes de ativar por padrão.

**Importante:** esta documentação registra o requisito e a ordem de trabalho; não significa que o lobby, Verity, as animações ou a cutscene já estejam implementados.
