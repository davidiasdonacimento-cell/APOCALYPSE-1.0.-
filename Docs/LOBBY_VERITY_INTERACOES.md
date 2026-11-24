# Lobby — Verity: proximidade, interações e cutscenes

**Status:** primeira versão do lobby de pré-partida implementada em Java; compilação e validação Android pendentes. A personagem atual é uma ilustração 2D temporária, não um modelo 3D animado.  
**Regra de integração:** NÃO adicionar esta funcionalidade ao aplicativo enquanto o lobby não estiver implementado, compilável e validado. Primeiro inspecionar a arquitetura e os componentes reais do lobby; depois integrar em módulos pequenos, sem substituir telas existentes.

## Objetivo
Permitir que o jogador se aproxime de Verity no lobby, permaneça ao lado dela e acione interações contextuais com animações e pequenas cutscenes.

## Fluxo confirmado pelo usuário\n1. Tocar em INICIAR JOGO no lobby principal.\n2. Exibir uma cutscene curta de entrada com Verity e opção de pular.\n3. Abrir o lobby pré-partida com Verity visível.\n4. Permitir enviar mensagens para Verity por meio da tela de conversa/IA e voltar ao lobby sem iniciar a partida.\n5. Permitir um gesto de aceno e, somente quando o jogador tocar em COMEÇAR PARTIDA, abrir a cena de jogo.\n\n## Requisitos funcionais
1. **Presença no lobby:** Verity deve aparecer como personagem separado, posicionada ao lado do ponto inicial do jogador.
2. **Proximidade:** detectar distância entre jogador e Verity; exibir o botão de interação somente dentro do alcance definido.
3. **Movimentação:** permitir que o jogador caminhe até ela usando os controles do lobby, sem teletransporte involuntário.
4. **Interações contextuais:** oferecer opções adequadas ao contexto, como conversar, acenar, abraço e beijo na bochecha. Não implementar beijo na boca nesta especificação.
5. **Consentimento e estado:** Verity deve aceitar, recusar ou encerrar cada interação de forma explícita e coerente com o estado atual; impedir sobreposição de animações.
6. **Animação:** criar estados separados para idle, olhar/virar para o jogador, caminhar, aproximação, abraço, gesto de carinho e retorno ao idle. Sincronizar transições com a cutscene.
7. **Cutscene curta:** câmera e personagens devem enquadrar a interação; permitir pular a cena; devolver o controle ao jogador ao terminar.
8. **Áudio e expressão:** expressões faciais, movimento de cabeça e falas/áudio opcionais devem acompanhar os eventos sem bloquear o jogo.
9. **Mensagens no lobby:** botão dedicado abre a conversa existente da Verity; ao voltar, permanecer no lobby pré-partida.\n10. **Android:** manter baixo consumo de memória e processamento; carregar animações sob demanda e evitar dependências pesadas.

## Arquitetura sugerida
- LobbyInteractionController: controla alcance, seleção e estado da interação.
- VerityCompanion: estado, posicionamento e resposta da personagem.
- InteractionAnimationController: toca e sincroniza animações.
- LobbyCutsceneController: câmera, duração, skip e devolução do controle.

Os nomes acima são propostas; adaptar aos nomes e padrões existentes depois de inspecionar o código real.

## Máquina de estados mínima
IDLE -> PLAYER_NEAR -> INTERACTION_MENU -> PLAYING_INTERACTION -> RETURN_TO_IDLE

A qualquer momento permitido, o jogador pode cancelar ou pular a cutscene. Ao sair do alcance antes de iniciar, fechar o menu. Não permitir iniciar outra interação enquanto uma estiver em execução.

## Implementação atual\n- `VerityLobbyActivity.java` criada com cutscene de entrada de 2,4 segundos e opção de pular.\n- Lobby pré-partida mostra uma ilustração 2D temporária de Verity, botão de mensagem, gesto de aceno e botão COMEÇAR PARTIDA.\n- INICIAR JOGO no menu principal abre `VerityLobbyActivity`; a partida só abre pelo botão COMEÇAR PARTIDA.\n- Mensagem abre `VeritySceneActivity`; voltar retorna ao lobby.\n- Modelo 3D, movimento livre do jogador até Verity, proximidade real e rig/animações 3D continuam pendentes.\n\n## Critérios de aceitação
- O lobby abre normalmente sem a funcionalidade ativada.
- Jogador e Verity aparecem em posições estáveis.
- O botão surge apenas no alcance correto.
- Cada opção toca a animação correspondente e termina no estado correto.
- Cutscene pode ser pulada e devolve o controle.
- Nenhuma animação fica travada ou sobreposta.
- Não há erros de compilação, JNI ou renderização.
- Testar em Android antes de marcar como concluído.

## Ordem de implementação
1. Inspecionar documentação e código do lobby existente.\n2. Implementar a primeira tela pré-partida e o fluxo de mensagens.
3. Confirmar sistema de personagem, animações, câmera e input já disponíveis.
4. Integrar posicionamento e detecção de proximidade.
5. Adicionar menu e máquina de estados.
6. Integrar animações e cutscene.
7. Compilar, testar no Android e corrigir antes de ativar por padrão.

**Importante:** esta documentação registra o requisito e a ordem de trabalho; não significa que o lobby, Verity, as animações ou a cutscene já estejam implementados.
