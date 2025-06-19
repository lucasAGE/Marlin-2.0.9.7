#include "../../inc/MarlinConfig.h"

#if HAS_HEATED_BED

#include "../gcode.h"
#include "../../module/temperature.h"
#include "../../lcd/marlinui.h"

/**
 * M140 / M190 – Set Bed Temperature
 *
 * M140: Define a temperatura da cama e retorna imediatamente.
 * M190: Define a temperatura da cama e aguarda até atingir o valor alvo.
 *
 * Parâmetros:
 *  I<index>  : Índice de predefinição de material (se aplicável)
 *  S<temp>   : Temperatura alvo em °C. Usado com M140 (sem espera) e com M190 (aguarda apenas aquecimento)
 *  R<temp>   : Temperatura alvo em °C. Usado com M190 (aguarda aquecimento ou resfriamento)
 *  P<bed>    : Índice da cama aquecida (0 a 3). Opcional; padrão é cama 0. Ignorado se MULTI_BED desativado.
 *
 * Exemplos:
 *  M140 S60         → Em multi‐bed, aplica 60 °C a todas as camas. Em single‐bed, aplica 60 °C à cama 0.
 *  M190 R40         → Aguarda a(s) cama(s) atingir(em) 40 °C
 *  M140 P2 S70      → Em multi‐bed, aplica apenas cama 2 = 70 °C. Em single‐bed, ignora P e aplica cama 0 = 70 °C.
 *  M190 P1 R90      → Em multi‐bed, aguarda cama 1 atingir 90 °C. Em single‐bed, ignora P e aguarda cama 0 = 90 °C.
 *
 * Observações:
 *  - Com PRINTJOB_TIMER_AUTOSTART ativado, M140 pode parar e M190 pode iniciar o cronômetro de impressão.
 *  - Se ENABLE_MULTI_HEATED_BEDS não estiver ativado, qualquer P<bed> é ignorado e atua sobre cama única (índice 0).
 */

void GcodeSuite::M140_M190(const bool isM190) {

  if (DEBUGGING(DRYRUN)) return;

  bool got_temp = false;
  celsius_t temp = 0;

  //#####################################################################################################
  //########################          TCC LUCAS          ################################################
  //#####################################################################################################

  // Em multi‐bed, checa se usuário forneceu 'P'; em single‐bed, P é ignorado e 'bed' sempre = 0.
  #if ENABLED(ENABLE_MULTI_HEATED_BEDS)
    const bool specific_bed = parser.seen('P');
    const uint8_t bed_index = specific_bed ? parser.value_byte() : 0;
    if (specific_bed && bed_index >= MULTI_BED_COUNT) return;
  #else
    constexpr bool specific_bed = false;
    constexpr uint8_t bed_index = 0;
  #endif

  // Se for preheat (I<index>), define a temperatura pela preset
  #if HAS_PREHEAT
    got_temp = parser.seenval('I');
    if (got_temp) {
      const uint8_t idx = _MIN(parser.value_byte(), PREHEAT_COUNT - 1);
      temp = ui.material_preset[idx].bed_temp;
    }
  #endif

  // Se não veio I<index>, procura S<temp> (para M140 e M190‐Aquec.) ou R<temp> (para M190)
  bool no_wait_for_cooling = false;
  if (!got_temp) {
    no_wait_for_cooling = parser.seenval('S');
    got_temp = no_wait_for_cooling || (isM190 && parser.seenval('R'));
    if (got_temp) temp = parser.value_celsius();
  }
  if (!got_temp) return;


  //#####################################################################################################
      //########################          TCC LUCAS          ################################################
      //#####################################################################################################
  // Se multi‐bed e sem P → alvo comum para todas. Se P ou single‐bed → alvo apenas para bed_index.
  #if ENABLED(ENABLE_MULTI_HEATED_BEDS)
    if (specific_bed) {
      thermalManager.set_specific_bed_target(bed_index, temp);
    } else {
      thermalManager.set_all_beds_target(temp);
    }
  #else
    thermalManager.setTargetBed(0, temp);
  #endif

  // Exibe mensagem “Bed Heating” ou “Bed Cooling” no LCD
  #if ENABLED(ENABLE_MULTI_HEATED_BEDS)
    if (specific_bed) {
      thermalManager.isHeatingBed(bed_index)
        ? LCD_MESSAGE(MSG_BED_HEATING)
        : LCD_MESSAGE(MSG_BED_COOLING);
    }
    else {
      // checa apenas cama 0 para decidir a mensagem, mas você poderia escolher outra lógica
      thermalManager.isHeatingBed(0)
        ? LCD_MESSAGE(MSG_BED_HEATING)
        : LCD_MESSAGE(MSG_BED_COOLING);
    }
  #else
    thermalManager.isHeatingBed(0)
      ? LCD_MESSAGE(MSG_BED_HEATING)
      : LCD_MESSAGE(MSG_BED_COOLING);
  #endif

  // Se PRINTJOB_TIMER_AUTOSTART estiver habilitado, atualiza cronômetro de impressão
  TERN_(PRINTJOB_TIMER_AUTOSTART, thermalManager.auto_job_check_timer(isM190, !isM190));

  // Se for um M190 (aguarda atingimento do alvo), entra no laço de espera:
  if (isM190) {
  #if ENABLED(ENABLE_MULTI_HEATED_BEDS)
    if (specific_bed) {
      thermalManager.wait_for_specific_bed(bed_index, no_wait_for_cooling);
    } else {
      // Chama _uma única vez_ a rotina que espera todas as camas
      thermalManager.wait_for_all_beds(no_wait_for_cooling);
    }
  #else
    thermalManager.wait_for_bed(0, no_wait_for_cooling);
  #endif
}
  // Se for apenas M140 (sem espera), definimos função de “status reset” para exibir o check‐mark
  else {
    #if ENABLED(ENABLE_MULTI_HEATED_BEDS)
      if (specific_bed) {
        // Ola em “status reset” só quando a cama específica atingir o alvo
        switch (bed_index) {
          case 0:
            ui.set_status_reset_fn([] {
              const celsius_t c = thermalManager.degTargetBed(0);
              return c < 30 || thermalManager.degBedNear(0, c);
            });
            break;
          case 1:
            ui.set_status_reset_fn([] {
              const celsius_t c = thermalManager.degTargetBed(1);
              return c < 30 || thermalManager.degBedNear(1, c);
            });
            break;
          case 2:
            ui.set_status_reset_fn([] {
              const celsius_t c = thermalManager.degTargetBed(2);
              return c < 30 || thermalManager.degBedNear(2, c);
            });
            break;
          case 3:
            ui.set_status_reset_fn([] {
              const celsius_t c = thermalManager.degTargetBed(3);
              return c < 30 || thermalManager.degBedNear(3, c);
            });
            break;
          default:
            ui.set_status_reset_fn(nullptr);
            break;
        }
      }
      else {
        // Genérico: só sai do “aguardar status reset” quando todas as camas estiverem a menos de 30 °C
        ui.set_status_reset_fn([] {
          bool all_ok = true;
          for (uint8_t b = 0; b < MULTI_BED_COUNT; b++) {
            const celsius_t c = thermalManager.degTargetBed(b);
            if (!(c < 30 || thermalManager.degBedNear(b, c))) {
              all_ok = false;
              break;
            }
          }
          return all_ok;
        });
      }
    #else
      // Single‐bed: sai do “aguardar status reset” quando cama 0 < 30 °C ou já perto do alvo
      ui.set_status_reset_fn([] {
        const celsius_t c = thermalManager.degTargetBed(0);
        return c < 30 || thermalManager.degBedNear(0, c);
      });
    #endif
  }
}

#endif // HAS_HEATED_BED
