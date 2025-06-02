#ifndef ADS1115_H
#define ADS1115_H

/**
 * ads1115.h
 *
 * Driver genérico para o conversor A/D I²C ADS1115, utilizado para ler
 * até 4 canais analógicos de alta resolução (16 bits). Ideal para ler
 * termistores de cama através de um divisor de tensão ou circuito amplificador.
 *
 * Funcionalidades básicas:
 *  - begin(): inicializa o barramento I²C, configura ganho (PGA) e taxa de amostragem
 *  - readRaw(channel): dispara uma conversão em um canal [0..3] e retorna o valor bruto (int16_t)
 *  - readVoltage(channel): converte o valor bruto em tensão (float, em volts)
 */

#include <Arduino.h>
#include <Wire.h>

class ADS1115 {
public:
  /**
   * Construtor.
   * @param address Endereço I²C do ADS1115 (0x48..0x4B, dependendo de ADDR pin).
   */
  explicit ADS1115(uint8_t address = 0x48);

  /**
   * Inicializa o hardware I²C (Wire) e configura o endereço do ADS1115.
   * Deve ser chamado antes de qualquer operação de leitura.
   *
   * @param wirePort Referência ao objeto TwoWire (por exemplo, Wire).
   *                 Se não for passado, usa Wire por padrão.
   * @param pga      Valor do Programmable Gain Amplifier (p.ex. 0 = ±6.144V, 1 = ±4.096V, etc.).
   *                 Consulte o datasheet para a correspondência exata.
   * @param dataRate Taxa de amostragem em SPS (p.ex. 128, 250, 475, 860).
   *                 Valores suportados: 8, 16, 32, 64, 128, 250, 475, 860.
   */
  void begin(TwoWire &wirePort = Wire, uint8_t pga = 1, uint16_t dataRate = 128);

  /**
   * Lê o valor bruto (16 bits) de um canal de entrada.
   * Faz uma conversão em modo “single-shot” (uma única leitura), aguarda a
   * conclusão e retorna o resultado em int16_t (com sinal).
   *
   * @param channel Índice do canal a ser lido [0..3].
   * @return Valor bruto de 16 bits (−32768 .. +32767).
   */
  int16_t readRaw(uint8_t channel);

  /**
   * Lê a tensão presente no canal especificado, convertendo o valor bruto
   * em volts de acordo com o PGA configurado.
   *
   * @param channel Índice do canal a ser lido [0..3].
   * @return Tensão em volts (p. ex. 0.000 .. 4.096 V).
   */
  float readVoltage(uint8_t channel);

private:
  TwoWire * _wire;       ///< Ponteiro para o objeto I²C (Wire)
  uint8_t   _address;    ///< Endereço I²C do ADS1115
  uint8_t   _pga;        ///< Código PGA: 0 → ±6.144V, 1 → ±4.096V, 2 → ±2.048V, 3 → ±1.024V, 4 → ±0.512V, 5 → ±0.256V
  uint16_t  _dataRate;   ///< Taxa de amostragem (SPS)

  // Registradores e máscaras conforme datasheet
  static constexpr uint8_t  REG_CONVERSION   = 0x00;
  static constexpr uint8_t  REG_CONFIG       = 0x01;
  static constexpr uint8_t  REG_LO_THRESH    = 0x02;
  static constexpr uint8_t  REG_HI_THRESH    = 0x03;

  // Bits de configuração (simplificado para single-shot)
  static constexpr uint16_t OS_SINGLE       = 0x8000; // Inicia uma conversão single-shot
  static constexpr uint16_t MUX_OFFSET      = 12;     // Bits 12..14 definem o canal
  static constexpr uint16_t PGA_OFFSET      = 9;      // Bits 9..11 definem o ganho
  static constexpr uint16_t MODE_SINGLE     = 0x0100; // Modo single-shot
  static constexpr uint16_t DR_OFFSET       = 5;      // Bits 5..7 definem a data rate
  static constexpr uint16_t COMP_QUE_DISABLE = 0x0003; // Desabilita comparador

  /**
   * Escreve um registrador de 16 bits no ADS1115.
   * @param reg  Endereço do registrador (0x00..0x03).
   * @param value Valor a ser escrito.
   */
  void writeRegister(uint8_t reg, uint16_t value);

  /**
   * Lê um registrador de 16 bits do ADS1115.
   * @param reg Endereço do registrador (0x00..0x03).
   * @return Valor lido (16 bits).
   */
  uint16_t readRegister(uint8_t reg);

  /**
   * Retorna o fator de conversão (LSB → volts) com base no PGA atual.
   * @return LSB_to_V = FS_range / 32768.0
   */
  float lsbToVoltage() const;
};

#endif // ADS1115_H
