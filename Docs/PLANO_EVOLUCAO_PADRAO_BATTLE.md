# Plano de evolução visual e jogável — APOCALYPSE

**Objetivo:** elevar o APOCALYPSE progressivamente ao padrão técnico e de acabamento observado no BATTLE UNIVERSE, mantendo identidade própria de sobrevivência zumbi. Este documento é um plano de engenharia; itens não são considerados implementados até existirem no código, compilarem e passarem por teste.

## Referência técnica auditada no BATTLE UNIVERSE

A inspeção do repositório BATTLE confirmou:
- Renderização 3D em perspectiva com matrizes de projeção, câmera e transformações.
- Malhas reutilizáveis (cubos, esferas, cilindros e cones), VBOs e normais para iluminação.
- Shaders com iluminação difusa e especular.
- Personagem com estados de animação procedurais e seleção visual.
- Controles multitoque com joystick, câmera independente e botões simultâneos.
- HUD posicionada em canvas de design 1280x720 com conversão de coordenadas para telas panorâmicas.
- Sistemas de partida, bots, coleta, estatísticas e configurações separados por estado.
- Pacote de conteúdo externo com verificação de integridade; o conteúdo só deve aumentar quando for realmente consumido pelo jogo.

O APOCALYPSE deve adaptar os padrões adequados, sem copiar contas, loja, regras de Battle Royale ou conteúdo exclusivo do BATTLE. O repositório BATTLE não deve ser alterado.

## Auditoria honesta do APOCALYPSE

- O renderer de gameplay atual usa projeção isométrica simplificada, geometria procedural e cores por vértice.
- A cidade ainda usa geometria procedural estilizada; não possui materiais PBR, modelos artísticos externos, colisões completas ou carregamento por chunks.
- O personagem jogável recebeu volumes orgânicos elipsoidais/cilíndricos para substituir a silhueta inteiramente cúbica; ainda não possui malha artística com rig esquelético nem animação de locomoção completa.
- Os controles atuais são um joystick e um botão de pulso; ainda não têm câmera independente e multitouch completo.
- A cena da Verity usa geometria procedural low-poly. A caminhada é demonstrativa no lugar; não existe skinning por pesos nem sincronização labial por fonemas.
- A sobrevivência está integrada ao código, mas ainda não passou por compilação e teste físico documentados.
- A primeira compilação de CI foi pequena e não representa conteúdo visual completo. Não aumentar tamanho artificialmente.

## Padrão de qualidade e regras

1. Cada sistema deve ter responsabilidade clara e interfaces pequenas.
2. Não colocar o jogo inteiro em um único arquivo.
3. Preservar orientação landscape e o fluxo lobby -> Verity -> partida.
4. Projetar para Android intermediário: limites de entidades, distância de renderização, LOD, carregamento por chunks e controle de memória.
5. Toda tela e controle deve funcionar em proporções panorâmicas diferentes; entrada multitoque deve permitir mover, girar a câmera e acionar controles simultaneamente.
6. Não adicionar assets de demonstração só para inflar APK. Cada asset precisa ser usado e ter licença/origem apropriada.
7. Não declarar uma etapa pronta por existir código. Exigir build CI, inspeção do APK, testes automatizados possíveis e teste físico no Redmi 12 quando disponível.
8. Preservar APIs, arquivos e dados existentes; antes de qualquer alteração, reler a documentação relevante e buscar os arquivos atuais no GitHub.

## Roadmap incremental

### Etapa A — fundação gráfica 3D
- Substituir a projeção isométrica por câmera em perspectiva e câmera de terceira pessoa.
- Criar biblioteca de matemática 3D e transformações compartilhadas.
- Criar malhas reutilizáveis com normais, buffers estáticos e shaders de iluminação.
- Separar renderização de terreno, cenário, personagem, entidades e HUD.
- Implementar ciclo de vida GL seguro (recriar recursos após perda do contexto).
**Aceitação:** cena em perspectiva estável, profundidade correta, sem vazamento de buffers, build ARM64/ARMv7.

### Etapa B — personagem e controles
- Modelo de personagem com proporções consistentes, orientação e animações idle/walk/run.
- Movimento relativo à câmera, rotação suave e colisão com cenário.
- Joystick esquerdo, câmera por arraste no lado direito e botões multitouch.
- HUD responsiva em canvas 1280x720, adaptada ao aspecto real da tela.
**Aceitação:** mover e girar a câmera ao mesmo tempo, sem perder toques; personagem não atravessa obstáculos.

### Etapa C — primeira região realmente jogável
- Metrópole com módulos de rua, calçada, edifícios, interiores selecionados e pontos de referência.
- Colisões e navegação separadas da malha visual.
- Sistema de chunks com carregamento/descarregamento por distância.
- Iluminação, neblina e atmosfera com qualidade configurável.
**Aceitação:** explorar a área sem atravessar prédios, sem carregar o mundo inteiro e sem travamentos prolongados.

### Etapa D — sobrevivência
- Zumbis com estados limitados e previsíveis, navegação simplificada e animações.
- Vida, recursos, coleta contextual, inventário e objetivos.
- Combate fictício com feedback não gráfico, cooldowns e balanceamento.
- Persistência local versionada.
**Aceitação:** ciclo completo de exploração, coleta, encontro, objetivo e retorno; sem duplicação de itens nem valores inválidos.

### Etapa E — lobby e Verity
- Lobby 3D com cenário, iluminação, câmera, personagem e interface landscape.
- Rig de personagem adequado, animações conectadas ao deslocamento e expressões básicas.
- Conversa com gateway, estados de carregamento/erro e retorno ao lobby sem perder contexto.
- Começar partida apenas por ação explícita.
**Aceitação:** abrir lobby, conversar, voltar e iniciar partida repetidamente sem crash ou perda de estado.

### Etapa F — conteúdo, otimização e acabamento
- Pacote de assets versionado com SHA-256, extração segura e verificação de espaço.
- LOD, frustum culling, instancing quando adequado, limites de memória e qualidade configurável.
- Áudio, opções gráficas, salvamento e telemetria local de desempenho sem dados sensíveis.
**Aceitação:** APK verificado, tamanho explicado pelo conteúdo real, desempenho medido e teste no aparelho.

## Estado inicial desta evolução

- Auditoria comparativa BATTLE/APOCALYPSE: concluída em 24/11/2026.
- Roadmap técnico: documentado.
- Etapa A — primeiro incremento de código enviado: `math3d.h` com matrizes 4x4, perspectiva, câmera look-at e translação; `game_engine.cpp` agora usa câmera de terceira pessoa em perspectiva e iluminação direcional calculada no shader.
- Etapa C — primeiro distrito urbano expandido no código: edifícios com fachadas/janelas/portas, variação de alturas e usos, anexo do hospital, parque com caminhos e árvores, postes, carros abandonados, marcações de pista, entulho e cercas procedurais.
- A expansão é geometria procedural consumida pelo renderer; não é um pacote de dezenas de MB e não deve ser confundida com assets artísticos finais.
- Ainda pendentes: modelos 3D artísticos, texturas de cenário, interiores realmente acessíveis, colisão de edifícios, chunks e validação de desempenho.
- Build CI após a expansão: aguardando resultado.
- Teste físico no Redmi 12: pendente.
- Incremento enviado: folhagem, postes, pneus e silhuetas de sobrevivente/zumbis passaram a usar malhas orgânicas trianguladas, reduzindo o aspecto de blocos.
- Colisão inicial enviada: edifícios registram volumes físicos AABB; movimento do jogador resolve os eixos separadamente e permite deslizar pelas fachadas. Ainda faltam colisões para vegetação, carros, cercas, destroços e interiores.
- Próximos incrementos: pipeline de assets com UVs e texturas reais, materiais, chunks/streaming e expansão de interiores; depois colisões completas e otimização de memória.


## Incremento de renderização com UV e materiais — 24/11/2026
- Enviado ao renderer: atributos de UV e ID de material no vértice; shader com seleção de textura por material; UV tiling em terreno e superfícies; três mapas iniciais tileáveis gerados em memória (grama/vegetação, concreto e asfalto), com mipmaps.
- Os padrões iniciais são 64 × 64 e não aumentam o tamanho do APK, pois são gerados na inicialização. Eles validam o caminho de amostragem, mas não são texturas fotográficas ou de alta resolução.
- Ainda pendentes para cumprir o alvo artístico: importador de modelos OBJ/glTF com buffers de índices/normais/UVs, carregamento de imagens reais de assets, materiais PBR (albedo, normal, roughness/metallic), modelos arquitetônicos de qualidade e revisão visual no aparelho.
- Build CI deste incremento: aguardando. Teste no Redmi 12: pendente.

## Importador OBJ/glTF e renderização PBR — 24/11/2026
- Criados os módulos `model_pipeline` e `model_renderer`, integrados ao CMake e ao renderer principal.
- OBJ/MTL: leitura de vértices, UVs, normais, índices de face, triangulação de polígonos e separação por material.
- glTF 2.0/GLB: leitura de primitivas triangulares, atributos POSITION/NORMAL/TEXCOORD_0, índices e fatores/texturas PBR baseColor, metallic-roughness e normal.
- Android AssetManager é passado pela JNI; buffers externos de glTF são lidos a partir do pacote assets.
- Renderer OpenGL: VBO/EBO, albedo, normal, mapa metallic-roughness e shader Cook–Torrance/GGX.
- Primeiro modelo OBJ de hospital adicionado e chamado na cena de gameplay.
- Fallback procedural compacto é usado somente quando os mapas não estão presentes. Não é correto descrever o asset atual como modelo fotorealista ou textura artística de alta resolução.
- Pendente: imagens artísticas reais, tratamento completo de transforms de nós glTF, transparência/cutout, validação de shaders em GPU real, medição de FPS/memória e teste no Redmi 12.


## Validação de compilação — 24/11/2026
- GitHub Actions: `assembleDebug` concluído com sucesso para o código nativo e Java integrado, commit `1512cc20c4ac90efbb0939b504403783f19e1131`.
- Artefato `apocalypse-debug-apk` gerado e publicado no workflow `45419387888`.
- Esta validação confirma compilação Android arm64-v8a; não comprova teste visual ou execução no Redmi 12. Teste físico e validação de shader em GPU permanecem pendentes.


## Incremento — câmera próxima, companheira e diagnóstico VERITY — 24/11/2026
- Implementada câmera em terceira pessoa próxima do jogador.
- Atualizado o modelo procedural do sobrevivente com peças visuais de equipamento e detalhes de rosto/roupa.
- VERITY passou a ser desenhada também dentro do render loop da partida, posicionada ao lado do jogador e com ciclo de caminhada.
- Reduzida a intensidade de iluminação do cenário e do shader PBR.
- Adicionado diagnóstico de saúde do gateway e leitura de mensagens HTTP reais na conversa da VERITY.
- Pendente: compilação deste incremento, inspeção da imagem no dispositivo, confirmação de que o endpoint/token/provedor estão corretos e substituição dos modelos procedurais por assets artísticos rigados.


## Incremento — fala ambiente e personagens volumétricos — 24/11/2026
- A partida agora agenda falas espontâneas da VERITY após 25 segundos e a cada 90 segundos, usando o HUD real como contexto e o gateway previamente configurado.
- Respostas são exibidas em legenda temporária e sintetizadas pelo Android TTS em português brasileiro; controle persistente permite desligar a voz.
- Requisições ocorrem em executor separado da renderização; pausa interrompe agenda e voz.
- A VERITY foi afastada para 1,72 unidade lateral e alterna idle/caminhada conforme o jogador se move.
- A malha da VERITY recebe iluminação volumétrica por normais derivadas.
- Zumbis passaram a ter malhas 3D anatômicas por partes e ciclo procedural simples de membros em patrulha/perseguição.
- **Não declarar personagens realistas concluídos:** jogador, VERITY e zumbis continuam low-poly procedurais. Skinning glTF, rig importado, blendshapes e assets artísticos permanecem pendentes.
- Pendente: build CI deste incremento, inspeção visual/sonora no Redmi 12 e ajuste de posição/legibilidade conforme o teste real.

## Ciclo procedural do sobrevivente — 24/11/2026
- O modelo 3D do jogador agora atualiza a malha durante o movimento e desloca alternadamente pernas, botas e braços para formar um ciclo simples de caminhada.
- Ao parar, a malha retorna à pose neutra. A atualização do buffer acontece somente durante o movimento e na transição para parado.
- Isso é animação procedural de peças volumétricas, não skinning por pesos de vértices nem animação mocap; rig esquelético completo permanece pendente.
