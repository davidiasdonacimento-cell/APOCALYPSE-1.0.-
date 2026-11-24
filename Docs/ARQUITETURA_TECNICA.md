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
- Projeto Android APOCALYPSE: **ainda não implementado**.
- Compilação local no Android: **pendente de prova**.
- APK: **não gerado**.
