# Fase 3 — Sistemas de sobrevivência

**Status:** planejamento técnico registrado; implementação ainda pendente de compilação e validação no Android.

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
- Implementação funcional da Fase 3: pendente.
- Compilação e teste no Android: pendentes.
