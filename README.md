# Bolsa de Valores

## Descrição do Projeto
Este projeto consiste na implementação de uma bolsa de valores simplificada, desenvolvida como parte do trabalho prático da unidade curricular de Sistemas Operativos 2 (SO2) do curso de Engenharia Informática. O sistema é composto por vários programas que interagem entre si para simular uma bolsa de valores, onde ações de empresas podem ser compradas e vendidas.

## Estrutura do Projeto
O sistema é composto por quatro programas principais:

1. **Bolsa**: O programa central que gere todas as operações e dados da bolsa de valores. Controla as empresas, os utilizadores, e as transações de compra e venda de ações.

2. **Cliente**: Interface que permite aos utilizadores interagir com a bolsa, enviando ordens de compra e venda, consultando o saldo, entre outras funcionalidades.

3. **Board**: Exibe as empresas com as ações mais valiosas, atualizando-se automaticamente com as últimas transações. Comunica com o programa Bolsa através de memória partilhada.

4. **BoardGUI (opcional)**: Semelhante ao Board, mas com uma interface gráfica que apresenta os valores das ações em formato de gráfico de barras.

## Funcionalidades Principais
- **Gerir Empresas**: Adicionar novas empresas, listar empresas existentes e ajustar preços de ações.
- **Gestão de Utilizadores**: Autenticação de utilizadores, consulta de saldo, e gestão de carteiras de ações.
- **Transações**: Comprar e vender ações, com impacto direto no saldo do utilizador e no preço das ações segundo a lei da oferta e da procura.
- **Pausa de Operações**: Permite pausar operações de compra e venda durante um período de tempo especificado.
- **Encerramento da Bolsa**: Fecha todos os programas e termina o sistema de forma ordenada.

## Estrutura de Dados e Comunicação
- **Bolsa – Cliente**: Comunicação via Named Pipes.
- **Bolsa – Board/BoardGUI**: Comunicação via Memória Partilhada.
- **Sincronização**: Utilização de mecanismos apropriados para garantir a consistência dos dados entre os processos.

## Requisitos
- **Ambiente de Desenvolvimento**: Windows com suporte para C/C++ e APIs de comunicação de processos.
- **Bibliotecas Necessárias**: Nenhuma biblioteca externa específica é necessária; utiliza-se apenas o API nativo do Windows.

## Instruções de Instalação e Execução
1. Clone o repositório do projeto:
   ```bash
   git clone <link_do_repositorio>
