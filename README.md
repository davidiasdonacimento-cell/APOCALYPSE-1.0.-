# APOCALYPSE 1.0

Projeto de jogo de sobrevivência zumbi em mundo aberto, desenvolvido com Unreal Engine 5.8 e com Android como plataforma-alvo.

> **Estado atual:** repositório inicializado com planejamento. Nenhuma compilação Unreal ou APK foi produzida ainda.

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
- Ter funcionamento híbrido: recursos locais/offline e IA generativa opcional por endpoint/modelo local compatível, sem serviço pago obrigatório.
- Usar TTS disponível no Android como opção de voz; reconhecer que TTS não é geração de diálogo.
- O reconhecimento de fala pelo microfone é opcional e deve ser implementado separadamente.

**Limite técnico explícito:** respostas offline pré-programadas são apenas fallback e não devem ser apresentadas como IA generativa completa. A geração local depende de um modelo/servidor disponível no dispositivo ou na rede.

### Lobby

- Abrir antes de qualquer partida.
- Não iniciar gameplay automaticamente.
- Ações: INICIAR JOGO, CONTINUAR, CONFIGURAÇÕES e PERSONALIZAR.
- Área de conversa com VERITY, com histórico e contexto editável.
- Persistência local de preferências e conversas.
- CONTINUAR só deve aparecer/funcionar quando houver progresso válido.
- INICIAR JOGO deve fazer a transição explícita para o mapa de gameplay.

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
| Survival | Necessidades e progressão de sobrevivência |
| Zombie | IA e comportamento dos inimigos |
| Combat | Combate, dano e armas |
| Inventory | Itens, coleta e armazenamento |
| Equipment | Slots e atributos de equipamentos |
| Lobby | Menu inicial e fluxo de entrada |
| VERITY | Companheira, diálogo, percepção e voz |
| Save | Persistência e migração de dados |
| UI | Interfaces e configurações |

## Diretrizes de engenharia

- Motor planejado: Unreal Engine 5.8.
- Plataforma-alvo: Android.
- Projetar para desempenho e memória limitados de dispositivos móveis.
- Preferir operações assíncronas para tarefas que possam bloquear o jogo.
- Validar dados antes de salvar ou carregar.
- Tratar indisponibilidade de rede, falhas de IA e falhas de áudio.
- Não incluir chaves privadas ou segredos no repositório.
- Não afirmar que um sistema está pronto sem implementação e verificação.
- Não afirmar que houve compilação, teste em aparelho ou geração de APK sem evidência real.
- Preservar sistemas existentes; mudanças destrutivas exigem autorização.

## Plano de desenvolvimento

1. **Fundação:** estrutura Unreal, módulos, configurações Android e verificação do ambiente de build.
2. **Lobby:** menu inicial, navegação, configurações e bloqueio de início automático.
3. **Gameplay básico:** personagem, mapa de teste, navegação e ciclo de jogo.
4. **Zumbis e combate:** IA, percepção, vida, dano e integração de armas.
5. **Inventário:** itens, coleta, armazenamento e equipamentos.
6. **VERITY:** companheira, percepção, suporte, diálogo, memória e voz.
7. **Integração:** fluxo lobby-gameplay, persistência, tratamento de falhas e otimização.
8. **Validação Android:** build real, instalação e testes no dispositivo-alvo.

## Critérios de conclusão

Um recurso só será considerado concluído quando:
- estiver implementado no projeto;
- compilar no ambiente Unreal 5.8 configurado;
- tiver comportamento verificado;
- tiver tratamento básico de falhas;
- não depender de um serviço pago obrigatório, quando isso for requisito;
- estiver testado no Android quando aplicável.

## Estado inicial

- Repositório novo confirmado.
- Planejamento registrado neste README.
- Código de jogo: ainda não adicionado.
- Ambiente de compilação Unreal/Android: ainda não validado.
- APK: ainda não gerado.
