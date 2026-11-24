# Arquitetura técnica inicial — APOCALYPSE 1.0

## Base confirmada

A inspeção do repositório `davidiasdonacimento-cell/BATTLE-UNIVERSE` confirmou:

- **Java** para a camada Android e integração com Activity, ciclo de vida, interface e APIs do sistema.
- **C++17** para o motor nativo.
- **OpenGL ES 3.0** para renderização 3D.
- **Gradle** para configurar e construir o aplicativo Android.
- **CMake 3.22.1** para compilar a biblioteca C++ nativa.
- **Android NDK 27.0.12077973** declarado no módulo Android do BATTLE.
- **Android compileSdk 35** declarado no módulo Android do BATTLE.

Arquivos inspecionados:
- `README.md`
- `build.gradle`
- `settings.gradle`
- `app/build.gradle`
- `app/src/main/AndroidManifest.xml`
- `app/src/main/java/com/battleuniverse/game/MainActivity.java`
- `app/src/main/java/com/battleuniverse/game/GameSurfaceView.java`
- `app/src/main/cpp/CMakeLists.txt`
- `app/src/main/cpp/game_engine.cpp`
- `.github/workflows/android.yml`

## Decisão de arquitetura

O APOCALYPSE seguirá a mesma família tecnológica nativa do BATTLE UNIVERSE, com código próprio e módulos separados por responsabilidade.

A reutilização será seletiva:
- Reaproveitar padrões comprovados de inicialização Android, superfície OpenGL, entrada multitoque e integração JNI quando forem adequados.
- Criar sistemas próprios para lobby, mundo de sobrevivência, zumbis, combate, inventário e VERITY.
- Não copiar funcionalidades, contas, loja, conteúdo ou regras de Battle Royale que não pertençam ao APOCALYPSE.
- Não alterar o repositório BATTLE UNIVERSE durante o desenvolvimento do APOCALYPSE.

## Estrutura lógica planejada

```text
app/
  src/main/
    java/<pacote-apocalypse>/
      MainActivity.java
      ui/
      lobby/
      settings/
      verity/
      storage/
    cpp/
      CMakeLists.txt
      core/
      render/
      world/
      survival/
      zombies/
      combat/
      inventory/
      equipment/
      verity/
```

A estrutura acima é o destino arquitetural, não uma declaração de que esses arquivos já foram criados.

## Ordem de implementação

1. Prova de compilação local no Android: validar ferramentas, SDK, NDK, CMake e Gradle disponíveis no próprio aparelho.
2. Fundação Android: Activity, ciclo de vida, janela de renderização OpenGL ES e camada JNI.
3. Core do jogo: estado, inicialização, pausa, retorno ao lobby e persistência local.
4. Lobby: menu funcional; nenhuma partida começa automaticamente.
5. Mundo e personagem: câmera, movimento, colisão e carregamento de cenário.
6. Zumbis e combate: percepção, navegação, vida, dano e suporte da VERITY.
7. Inventário e equipamentos: coleta, slots, baús e salvamento.
8. VERITY: diálogo generativo opcional, memória controlada pelo jogador, voz PT-BR e comportamento de companheira.
9. Integração, otimização e testes no dispositivo.

## Restrição de compilação — sem PC

O objetivo declarado é compilar no próprio Android. Isso ainda precisa ser demonstrado.

O workflow encontrado no BATTLE usa `runs-on: ubuntu-latest`; portanto, ele compila em um runner Linux hospedado pelo GitHub. Isso não comprova compilação no celular e não será apresentado como se comprovasse.

Antes de ampliar o projeto, a primeira prova técnica deverá compilar um aplicativo Android mínimo diretamente no aparelho. Se a combinação de Gradle, Android SDK, NDK e CMake não funcionar no ambiente móvel disponível, a limitação será informada antes de continuar.

## Estado desta documentação

- Base tecnológica do BATTLE: **inspecionada nos arquivos listados**.
- Reutilização de código: **ainda não realizada**.
- Projeto Android APOCALYPSE: **estrutura e código inicial presentes** (lobby de menu, Activity de Verity e cena nativa).
- Compilação local no Android: **pendente de prova**.
- Sistemas de sobrevivência da Fase 3: **código implementado e integrado; compilação/teste pendentes**.
- Lobby 3D com Verity e interações animadas: **pendente**.
- APK: **não gerado**.


## Pipeline inicial de materiais e UV — 24/11/2026
- O vértice do renderer recebeu coordenadas UV e um identificador de material, mantendo posição e cor.
- O shader OpenGL ES 3.0 recebe UV/material e seleciona entre três mapas tileáveis: vegetação, concreto e asfalto.
- O terreno e as superfícies planas recebem UVs proporcionais à dimensão para evitar esticar uma textura sobre áreas grandes.
- Os três mapas iniciais são padrões compactos gerados em memória no arranque; não adicionam arquivos de imagem nem bytes artificiais ao APK.
- Malhas sem material continuam no slot neutro para preservar personagens, zumbis e props existentes.
- Limitação explícita: esses mapas são texturas procedurais iniciais de 64 × 64, não fotografias nem texturas artísticas de alta resolução. Ainda não existe importador OBJ/glTF, leitura de imagens externas, mapas normais/roughness/metallic ou materiais PBR completos.
 
## Integração do importador e renderer PBR — 24/11/2026
- `model_pipeline.cpp` passou a compilar junto do motor; `cgltf` atende glTF 2.0/GLB e o leitor OBJ processa posições, UVs, normais, faces trianguladas e grupos de materiais MTL.
- O Android `AssetManager` é encaminhado pelo JNI ao renderer nativo para abrir assets empacotados.
- `model_renderer.cpp` cria VBO/EBO por submalha, carrega as texturas referenciadas quando disponíveis e desenha a geometria importada com shader PBR Cook–Torrance/GGX (albedo, normal, roughness e metallic).
- Foi adicionado um primeiro modelo OBJ de hospital, com submateriais de concreto, vidro e metal, incluído em `app/src/main/assets/models/`.
- Quando uma textura não existe, o renderer cria mapa compacto de fallback em memória; isso permite visualizar o pipeline sem adicionar imagens vazias ao APK.
- Limitações: o modelo de hospital é um primeiro asset de integração, não um edifício artístico de alta fidelidade; ainda faltam mapas artísticos de alta resolução, transparência de vidro, tratamento completo de transformações hierárquicas/skinning e teste visual no Redmi 12.
- Compilação CI da integração e validação em dispositivo devem ser registradas separadamente.


## Validação de compilação — 24/11/2026
- GitHub Actions: `assembleDebug` concluído com sucesso para o código nativo e Java integrado, commit `1512cc20c4ac90efbb0939b504403783f19e1131`.
- Artefato `apocalypse-debug-apk` gerado e publicado no workflow `45419387888`.
- Esta validação confirma compilação Android arm64-v8a; não comprova teste visual ou execução no Redmi 12. Teste físico e validação de shader em GPU permanecem pendentes.


## Ajustes de câmera, personagem e iluminação — 24/11/2026
- A câmera foi reposicionada para terceira pessoa próxima, atrás e ligeiramente deslocada do jogador, com alvo próximo à altura do tronco.
- A iluminação difusa do cenário e a contribuição ambiente/direta do shader PBR foram reduzidas para conter o excesso de claridade.
- O modelo procedural do jogador recebeu camadas de roupa de sobrevivência, colete, bolsos, mochila, botas, cabelo e traços faciais simplificados.
- A VERITY foi conectada ao render loop da partida como companheira ao lado do jogador.
- Essas alterações precisam de inspeção visual no Redmi 12; não representam ainda um personagem artístico de alta fidelidade nem uma validação de FPS.

## Diagnóstico da API VERITY — 24/11/2026
- A tela de conversa agora normaliza o endereço do gateway para `/v1/chat/completions` quando o usuário informa apenas o domínio Render.
- Foi adicionado teste de `/health` que mostra separadamente presença do modelo/URL, chave do provedor e token do gateway.
- Respostas HTTP de erro agora leem o corpo de erro do servidor e mostram status e mensagem úteis, inclusive distinção para 401 e 503.
- Nenhuma chave privada é embutida no APK; o teste real depende das variáveis privadas do serviço Render e do token correspondente no aparelho.


## Integração de voz ambiente da VERITY na partida — 24/11/2026
- `NativeGameActivity` reutiliza as preferências de endpoint e token configuradas no lobby/conversa; nenhum segredo do provedor foi embutido no código.
- Requisição HTTP ocorre em executor de fundo, não na thread de renderização nem na UI.
- A IA recebe o texto do HUD nativo como contexto factual e produz uma fala curta; o ciclo inicia após 25 s e repete a cada 90 s durante a Activity.
- Android TextToSpeech sintetiza em `pt-BR`; há controle persistente para ligar/desligar voz e legenda temporária.
- Em pausa, os callbacks periódicos são removidos e a fala é interrompida; em destruição, executor e TTS são encerrados.
- A VERITY tem offset lateral de 1,72 unidade e animação idle/walk de acordo com o movimento do jogador.
- O shader da malha da VERITY usa normal derivada de `dFdx/dFdy` para iluminação volumétrica sem textura 2D plana.
- O renderizador de sobrevivência agora constrói zumbis por partes volumétricas 3D (pelve, tronco, cabeça, braços, pernas e botas) e aplica oscilação de membros orientada pela fase de patrulha/perseguição.

## Limites de personagem e validação
- Os modelos atuais são geometria procedural low-poly, não assets humanos de alta fidelidade.
- Skinning de glTF, hierarquia de nós, pesos de vértices, blendshapes e animações importadas continuam pendentes.
- A animação do jogador ainda não foi convertida para um rig esquelético completo; a oscilação dos zumbis é procedural simplificada.
- Build Android, logs de execução, TTS, resposta real do gateway e renderização em GPU precisam ser validados separadamente; CI não substitui teste físico.
