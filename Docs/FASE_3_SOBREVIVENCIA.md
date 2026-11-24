# Fase 3 — Sistemas de sobrevivência

**Status:** sistemas implementados no código e integrados ao motor; compilação e validação no Android ainda pendentes.

## Objetivo
Adicionar um ciclo básico de sobrevivência ao APOCALYPSE sem comprometer o desempenho do Android. Os sistemas devem ser modulares e não podem ser tratados como concluídos antes de compilar e testar no aparelho.

## Ordem de implementação

### 1. Zumbis — percepção e comportamento (primeiro módulo)
- Entidade de zumbi com posição, estado e atributos simples.
- Estados: parado, patrulha curta, alerta, perseguição e retorno à área.
- Detecção do jogador por distância e campo de visão simplificado.
- Atualização de movimento com delta time e limite de velocidade.
- Limite de entidades ativas e distância máxima de simulação para poupar CPU.
- Separar lógica de IA, dados da entidade e renderização.
- Comportamento fictício e não gráfico; sem detalhes explícitos de ferimentos.

### 2. Vida e dano do jogador
- Atributos de vida e estado de incapacitação abstrato.
- Receber dano apenas por eventos de jogo validados e com intervalo de proteção contra repetição.
- Interface de vida legível e feedback visual não gráfico.
- Reinício/retorno ao lobby conforme regras definidas posteriormente.

### 3. Recursos e coleta
- Itens simples do mundo: água, alimento, materiais e suprimentos médicos fictícios.
- Interação por proximidade e botão contextual.
- Quantidades limitadas, validação de inventário e feedback de coleta.
- Não permitir duplicação ou valores negativos.

### 4. Combate fictício e equipamentos
- Definir primeiro regras abstratas de alcance, tempo de ação, recarga e consumo de recursos.
- Integrar ações e animações sem instruções ou simulação realista de uso de armas.
- Feedback visual não gráfico e balanceamento para dispositivos móveis.

## Arquitetura prevista
- `ZombieSystem`: ciclo de atualização e limite de entidades.
- `ZombieEntity`: estado e atributos individuais.
- `SurvivalStats`: vida e atributos do jogador.
- `WorldPickupSystem`: itens coletáveis e validação de proximidade.
- `CombatSystem`: resolução abstrata de ações, dano e tempos de espera.

Os nomes são propostas e devem ser adaptados à estrutura real existente após inspeção.

## Regras de integração
1. Consultar este documento, `README.md` e `Docs/ARQUITETURA_TECNICA.md` antes de cada alteração relacionada à sobrevivência.
2. Inspecionar os arquivos atuais antes de editar; não assumir que a estrutura planejada já existe.
3. Não iniciar a Fase 3 acoplando tudo em um único arquivo monolítico.
4. Preservar o lobby e a cena atual; não iniciar gameplay automaticamente.
5. Manter limites de processamento, memória e entidades ativos adequados ao Android.
6. Não afirmar que IA, combate, sobrevivência ou APK estão prontos sem compilação e teste reais.

## Critérios de aceitação da Fase 3
- IA de zumbis executa estados previsíveis sem travar o ciclo de renderização.
- Entidades respeitam limites de distância e quantidade.
- Vida e dano atualizam corretamente e sem valores inválidos.
- Itens podem ser coletados sem duplicação.
- Sistemas são independentes e integrados por interfaces claras.
- Projeto compila e é testado no Android antes de marcar cada módulo como concluído.

## Estado atual
- Requisitos gerais de zumbis, combate, dano e vida: descritos no `README.md`.
- Ordem e critérios da Fase 3: documentados neste arquivo.
- Módulo C++ `survival_system.h/.cpp`: criado e integrado ao motor nativo.
- Zumbis: quatro entidades iniciais, estados de aproximação/perseguição e ataque com intervalo.
- Vida: 100 pontos iniciais, dano abstrato e recuperação por kit médico.
- Recursos: água, comida, materiais e kits médicos; coleta automática por proximidade.
- Combate: ação fictícia de pulso com alcance, recarga e três pontos de resistência por zumbi.
- Renderização: zumbis e itens desenhados no cenário; HUD Android mostra vida, quantidade de zumbis e recursos; botão PULSO aciona o combate.
- Integração CMake: `survival_system.cpp` incluído na biblioteca nativa.
- Implementação de código da Fase 3: enviada ao GitHub; ainda não validada por compilação.
- Compilação e teste no Android: pendentes.


## Atualização visual dos zumbis — 24/11/2026
- A renderização não usa sprites PNG para os zumbis do sistema de sobrevivência: as entidades são geradas como malhas volumétricas no C++.
- O corpo foi dividido em volumes 3D para pelve, tronco, pescoço, cabeça, ombros, braços, mãos, pernas e botas.
- Olhos, boca e detalhes do casaco melhoram a leitura visual de frente.
- A fase procedural avança tanto em patrulha quanto em perseguição e anima braços/pernas com uma oscilação simples.
- Limitação: isso ainda é um modelo low-poly procedural, não um personagem escaneado ou asset artístico rigado. Skinning e animação esquelética completa continuam pendentes.
- A nova malha precisa de compilação e teste visual no Redmi 12 antes de ser considerada validada.


## Economia e loja durante a partida — 24/11/2026
- Cada zumbi derrotado concede $30 uma única vez, quando sua vida chega a zero.
- A loja da partida oferece cura imediata (+40 de vida por $50), melhoria de pulso (+1 de dano por $120, até nível 3) e colete (+2 de redução do dano recebido por nível, por $90, até nível 3).
- As compras são validadas no módulo nativo; saldo insuficiente e níveis máximos recusam a transação. O HUD mostra o saldo.
- Saldo e melhorias são mantidos apenas durante a sessão atual; persistência entre partidas/fechamento ainda não foi implementada.
- A quantidade inicial continua limitada às quatro entidades existentes; não há reaparecimento de zumbis nesta etapa.
- A loja foi integrada à Activity Android por JNI; compilação e teste real no Redmi 12 ainda são necessários.


## Ondas e loja de armas — 24/11/2026
- A primeira onda contém quatro zumbis. Ao eliminar todos, a próxima onda começa automaticamente com quantidade crescente até o limite de 12 inimigos ativos.
- A cada onda concluída, o jogador recebe bônus de dinheiro (US$ 100 + US$ 25 por número da nova onda), um kit médico e cinco shurikens de reserva.
- Os zumbis das ondas posteriores recebem resistência crescente em etapas; o sistema limita a quantidade simultânea para controlar custo de CPU.
- Loja: desbloqueio de shurikens ($100, 12 unidades), rifle ($260, 30 munições) e escopeta ($220, 12 cartuchos); recargas custam $45, $80 e $70, respectivamente.
- Armas podem ser selecionadas no painel da loja. O botão de combate usa a arma selecionada: pulso de curto alcance, shuriken de alcance médio, rifle de longo alcance ou escopeta de alcance curto e dano maior.
- A resolução atual é arcade instantânea por alcance/alvo mais próximo; projéteis 3D visíveis, trajetória física, animações de arremesso/recuo e efeitos de impacto ainda não foram implementados.
- Ondas, saldo, armas e munições ainda não são persistidos entre partidas.
- Compilação e teste no Redmi 12 permanecem pendentes.
