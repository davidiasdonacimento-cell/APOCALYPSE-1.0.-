# APOCALYPSE 1.0

Jogo de sobrevivência zumbi para Android, com mundo explorável, combate, coleta de recursos, inventário, equipamentos e uma companheira de IA chamada VERITY.

> **Estado real do projeto:** este repositório contém planejamento. Ainda não há implementação de gameplay, compilação validada ou APK.

## Direção técnica — corrigida

- **Não usar Unreal Engine.** A referência a Unreal Engine 5.8 que apareceu na primeira versão deste README foi um erro e está descartada.
- O objetivo é desenvolver e compilar para Android, priorizando o fluxo de trabalho pelo próprio celular, sem exigir um PC.
- A auditoria do repositório BATTLE-UNIVERSE confirmou a base técnica: Java na camada Android, C++17 no motor nativo, OpenGL ES 3.0 para renderização, Gradle para o projeto Android e CMake para compilar o código C++.
- O APOCALYPSE deve aproveitar essa mesma família tecnológica, adaptando os módulos ao jogo de sobrevivência; não copiar o jogo BATTLE inteiro nem substituir seus arquivos.
- A compilação no próprio celular ainda não foi validada. O workflow existente do BATTLE usa `ubuntu-latest` no GitHub Actions, o que é compilação em nuvem, não compilação no aparelho. Precisamos provar o fluxo local Android antes de prometer APK.

## Visão do jogo

APOCALYPSE é uma experiência de sobrevivência com exploração, combate, coleta de recursos, inventário, equipamentos e uma companheira de IA chamada VERITY.

A experiência deve começar sempre em um lobby. A partida só começa quando o jogador selecionar **INICIAR JOGO**.

## Requisitos centrais

### VERITY — companheira inteligente

- Acompanhar o jogador durante a exploração.
- Detectar ameaças e prestar suporte durante os confrontos.
- Iniciar conversas espontaneamente, sem exigir que o jogador abra o chat.
- Conversar em português brasileiro por voz e texto.
- Ter personalidade consistente e memória persistente, controlada pelo jogador.
- Considerar o contexto do jogo e reagir aos acontecimentos.
- Exibir as mensagens na interface.
- Permitir editar o contexto pessoal e controlar histórico/memórias.
- Oferecer configurações de voz, volume e conversas automáticas.
- Ter funcionamento híbrido: recursos locais/offline e IA generativa opcional por modelo local ou endpoint compatível, sem serviço pago obrigatório.
- Usar TTS disponível no Android como opção de voz; TTS não é geração de diálogo.
- O reconhecimento de fala pelo microfone é opcional e deve ser implementado separadamente.

**Limite técnico:** respostas offline pré-programadas são apenas fallback e não devem ser apresentadas como IA generativa completa. A geração local depende de um modelo/servidor disponível no dispositivo ou na rede.

### Lobby

- Abrir antes de qualquer partida.
- Não iniciar gameplay automaticamente.
- Ações: INICIAR JOGO, CONTINUAR, CONFIGURAÇÕES e PERSONALIZAR.
- Área de conversa com VERITY, com histórico e contexto editável.
- Persistência local de preferências e conversas.
- CONTINUAR só deve aparecer/funcionar quando houver progresso válido.
- INICIAR JOGO deve fazer a transição explícita para o gameplay.

### Gameplay e sobrevivência

- Mundo explorável com áreas e pontos de interesse.
- Personagem controlável e câmera apropriada para a experiência.
- Zumbis com percepção, navegação e comportamento de ameaça.
- Sistema de combate, dano e vida.
- Armas e equipamentos integrados ao gameplay.
- Coleta de recursos e itens.
- VERITY integrada à navegação, percepção de ameaças e suporte ao jogador.

### Inventário e equipamentos

- Inventário funcional com coleta, movimentação e uso de itens.
- Baús e armazenamento.
- Equipar e desequipar itens.
- Slots planejados: cabeça, corpo, mochila e utilitário.
- Dados validados e persistidos de forma segura.

## Arquitetura planejada

Manter módulos separados por responsabilidade, evitando tanto um arquivo monolítico quanto fragmentação desnecessária.

| Módulo | Responsabilidade |
|---|---|
| Core | Inicialização, ciclo de vida e configuração |
| World | Mundo, mapas e exploração |
| Survival | Sistemas de sobrevivência e progressão |
| Zombie | IA e comportamento dos inimigos |
| Combat | Combate, dano e armas |
| Inventory | Itens, coleta e armazenamento |
| Equipment | Slots e atributos de equipamentos |
| Lobby | Menu inicial e fluxo de entrada |
| VERITY | Companheira, diálogo, percepção e voz |
| Save | Persistência e migração de dados |
| UI | Interfaces e configurações |

## Plano de desenvolvimento

1. **Auditoria da base:** inspecionar o repositório BATTLE UNIVERSE para identificar a tecnologia real e o que pode ser reutilizado sem quebrar o projeto original.
2. **Prova de compilação Android:** validar uma compilação mínima pelo próprio celular, sem PC, antes de expandir o código.
3. **Fundação:** organizar módulos, inicialização, configurações e persistência.
4. **Lobby:** menu inicial, navegação, configurações e bloqueio de início automático.
5. **Gameplay básico:** personagem, mundo, navegação e ciclo de jogo.
6. **Zumbis e combate:** IA, percepção, vida, dano e integração de armas.
7. **Inventário:** itens, coleta, armazenamento e equipamentos.
8. **VERITY:** companheira, percepção, suporte, diálogo, memória e voz.
9. **Integração e otimização:** estabilidade, memória, desempenho e tratamento de falhas.
10. **Validação final:** compilação real, instalação e testes no Android.

## Diretrizes de engenharia

- Priorizar compatibilidade com Android e compilação pelo celular.
- Não trocar o motor/base do BATTLE por suposição; primeiro inspecionar o código real.
- Projetar para desempenho e memória limitados de dispositivos móveis.
- Preferir operações assíncronas para tarefas que possam bloquear o jogo.
- Validar dados antes de salvar ou carregar.
- Tratar indisponibilidade de rede, falhas de IA e falhas de áudio.
- Não incluir chaves privadas ou segredos no repositório.
- Não afirmar que um sistema está pronto sem implementação e verificação.
- Não afirmar que houve compilação, teste em aparelho ou geração de APK sem evidência real.
- Preservar sistemas existentes; mudanças destrutivas exigem autorização.

## Critérios de conclusão

Um recurso só será considerado concluído quando estiver implementado, integrado, compilado e testado no ambiente-alvo, com tratamento básico de falhas. A compilação pelo celular deve ser demonstrada antes de assumir que o fluxo é viável.

## Estado atual verificado no repositório

- Repositório APOCALYPSE 1.0 criado e com código Android/NDK presente.
- Base declarada: Java + C++17 + OpenGL ES 3.0 + Gradle + CMake.
- Cena nativa contém um blockout urbano procedural, personagem de teste, joystick e câmera de acompanhamento.
- Fase 3 tem módulo C++ separado para zumbis, estados de patrulha/perseguição, vida, coleta de recursos e combate arcade fictício; HUD e botão de pulso foram ligados à Activity Android.
- `Verity3DActivity` e `verity_3d.cpp` adicionam uma primeira personagem low-poly procedural em OpenGL ES 3.0, com câmera em perspectiva e lobby pré-partida. Rig procedural por juntas, caminhada demonstrativa, aceno, piscar e boca animada para fala já foram adicionados ao código. Skinning por pesos, deslocamento real, expressões avançadas e validação no aparelho continuam pendentes.
- Verity tem uma Activity separada de conversa/voz; não é uma personagem 3D animada.
- Proximidade real, movimento até Verity, rig e animações 3D completas continuam pendentes conforme Docs/LOBBY_VERITY_INTERACOES.md.
- Compilação Android, teste no aparelho e APK: ainda não validados/gerados.
