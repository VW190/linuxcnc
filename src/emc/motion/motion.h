/********************************************************************
* Description: motion.h
*   Data structures used throughout emc2.
*
* Author:
* License: GPL Version 2
* System: Linux
*
* Copyright (c) 2004 All rights reserved
********************************************************************/

/* jmk dice: ¡Este archivo es un desastre! */

/*

Divagaciones varias:

Los términos eje y articulación se utilizan de manera inconsistente en todo EMC.
Para todo código nuevo, los usos son los siguientes:

axis – uno de los nueve grados de libertad, x, y, z, a, b, c, u, v, w. Estos se refieren a los ejes en el espacio cartesiano, que pueden coincidir o no con las articulaciones (ver más abajo). En las máquinas cartesianas coinciden, pero no en los hexápodos, robots y otras máquinas no cartesianas.
Articulación – uno de los grados físicos de libertad de la máquina. Pueden ser lineales (husillos de avance) o rotatorios (mesas rotatorias, articulaciones de brazos robóticos). Puede haber cualquier cantidad de articulaciones. El código cinemático es responsable de traducir desde el espacio de ejes al espacio de articulaciones y viceversa.

Hay tres tipos principales de datos que necesita un controlador de movimiento:

1) datos compartidos con elementos de nivel superior: comandos, estado, etc.
2) datos que son locales al controlador de movimiento
3) datos compartidos con elementos de nivel inferior: pines hal

Además, algunos datos internos (2) se deben compartir para fines de resolución de problemas, aunque sean “internos” al controlador de movimiento. Según el tipo de datos, se pueden tratar como tipo (1) y ponerlos a disposición del código de nivel superior, o se pueden tratar como tipo (3) y ponerlos a disposición de hal, de modo que halscope pueda monitorearlos.

Este archivo SÓLO debe contener estructuras y declaraciones para elementos de tipo (1): aquellos que se comparten con código de nivel superior.

Los elementos de tipo (2) deben declararse en mot_priv.h, junto con los elementos de tipo (3).

Con el fin de mantener mi cordura, no voy a intentar mover todo a su ubicación adecuada todavía...

Sin embargo, todos los elementos nuevos se definirán en el lugar adecuado y algunos elementos existentes podrán moverse de una definición de estructura a otra.

*/

#ifndef MOTION_H
#define MOTION_H

#include "posemath.h"		/* PmCartesian, PmPose, pmCartMag() */
#include "emcpos.h"			/* EmcPose */
#include "cubic.h"			/* CUBIC_STRUCT, CUBIC_COEFF */
#include "emcmotcfg.h"		/* EMCMOT_MAX_JOINTS */
#include "kinematics.h" 	/* tipos de cinematicas */
#include "simple_tp.h"  	/* planificador de trayectoria para un solo eje */
#include "rtapi_limits.h" 	/* #if defined(__KERNEL__).... */
#include <stdarg.h>     	/*POSIX. La familia va_xxx */ 
#include "rtapi_bool.h" 	/* <stdbool.h> en C++ */
#include "state_tag.h"  	/* La familia GM_xxx */
#include "tp_types.h"   	/* La familia TP_xxx */

// Definir un valor especial para indicar un ID de movimiento no válido
// Nota: nunca genere un ID de movimiento MOTION_INVALID_ID
// esto realmente debería probarse en command.c

#define MOTION_INVALID_ID INT_MIN
#define MOTION_ID_VALID(x) ((x) != MOTION_INVALID_ID)

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct _EMC_TELEOP_DATA {
	EmcPose currentVel;
	EmcPose currentAccel;
	EmcPose desiredVel;
	EmcPose desiredAccel;
    } EMC_TELEOP_DATA;

/* Esta enum lista todos los comandos posibles */

    typedef enum {
	EMCMOT_ABORT = 1,  /* abortar todo movimiento */
	EMCMOT_ENABLE,     /* habilitar servos para articulaciones activas */
	EMCMOT_DISABLE,    /* deshabilitar servos para articulaciones activas */

	EMCMOT_PAUSE,      /* pausar el movimiento */
	EMCMOT_REVERSE,    /* ejecutar movimiento inverso */
	EMCMOT_FORWARD,    /* ejecutar movimiento inverso */
	EMCMOT_RESUME,     /* reanudar movimiento */
	EMCMOT_STEP,       /* reanudar movimiento hasta que se encuentre el id */
	EMCMOT_FREE,       /* establece modo en movimiento libre (articular) */
	EMCMOT_COORD,      /* establece modo en movimiento coordinado */
	EMCMOT_TELEOP,     /* establece modo en teleop */

	EMCMOT_SPINDLE_SCALE,	/* establece el factor de escala para la velocidad del husillo */
	EMCMOT_SS_ENABLE,		/* habilitar/deshabilitar el escalado de la velocidad del husillo */
	EMCMOT_FEED_SCALE,		/* establece el factor de escala para la velocidad de avance */
	EMCMOT_RAPID_SCALE,		/* establece el factor de escala para rápidos */
	EMCMOT_FS_ENABLE,		/* habilitar/deshabilitar la velocidad de avance de escala */
	EMCMOT_FH_ENABLE,		/* habilitar/deshabilitar feed_hold */
	EMCMOT_AF_ENABLE,		/* habilitar/deshabilitar velocidad de avance adaptativa */
	EMCMOT_OVERRIDE_LIMITS,	/* ignorar temporalmente los límites hasta que finalice el desplazamiento */

	EMCMOT_SET_LINE,			/* poner en cola un movimiento lineal */
	EMCMOT_SET_CIRCLE,			/* poner en cola un movimiento circular */
	EMCMOT_SET_TELEOP_VECTOR,	/* Moverse a una velocidad determinada pero en coordenadas cartesianas universales, 
									no en el espacio de articulaciones como EMCMOT_JOG_* */
	EMCMOT_CLEAR_PROBE_FLAGS,	/* borra el indicador probeTripped */
	EMCMOT_PROBE,				/* ir a pos, detener si la sonda se activa, grabar pos de activación */
	EMCMOT_RIGID_TAP,			/* ir a pos, con sincronización con velocidad del husillo, luego vuelve a la posición inicial */

	EMCMOT_SET_VEL,				/* establece la velocidad para los movimientos subsiguientes */
	EMCMOT_SET_VEL_LIMIT,		/* establece la velocidad máxima para todos los movimientos (tooltip) */
	EMCMOT_SET_ACC,				/* establece la aceleración máxima para los movimientos (tooltip) */
	EMCMOT_SET_TERM_COND,		/* establecer condición de terminación (stop, blend) */
	EMCMOT_SET_NUM_JOINTS,		/* establece el número de articulaciones */
	EMCMOT_SET_NUM_SPINDLES,	/* establece el número de husillos */
	EMCMOT_SET_WORLD_HOME,		/* establecer la pose para el home universal */

	EMCMOT_SET_DEBUG,			/* establece el nivel de depuración */
	EMCMOT_SET_DOUT,			/* establece o anula un DIO, esto puede ser inmediato o sincronizado con el movimiento */
	EMCMOT_SET_AOUT,			/* establece o anula un AIO, esto puede ser inmediato o sincronizado con el movimiento */
	EMCMOT_SET_SPINDLESYNC,		/* sincronizar el movimiento con el encoder del husillo */
	EMCMOT_SPINDLE_ON,			/* arrancar el husillo */
	EMCMOT_SPINDLE_OFF,			/* detener el husillo */
	EMCMOT_SPINDLE_INCREASE,	/* husillo más rápido */
	EMCMOT_SPINDLE_DECREASE, 	/* husillo más lento */
	EMCMOT_SPINDLE_BRAKE_ENGAGE,	/* activa el freno del husillo */
	EMCMOT_SPINDLE_BRAKE_RELEASE,	/* libera el freno del husillo */
	EMCMOT_SPINDLE_ORIENT,			/* orientar el husillo */
	EMCMOT_SET_OFFSET,				/* establecer compensaciones de herramientas */
	EMCMOT_SET_MAX_FEED_OVERRIDE,
	EMCMOT_SETUP_ARC_BLENDS,

	EMCMOT_SET_PROBE_ERR_INHIBIT,
	EMCMOT_ENABLE_WATCHDOG,			/* habilitar el sonido del watchdog, parport */
	EMCMOT_DISABLE_WATCHDOG,		/* deshabilitar sonido del watchdog, parport */
	EMCMOT_JOG_CONT,				/* jog continuo */
	EMCMOT_JOG_INCR,				/* jog incremental */
	EMCMOT_JOG_ABS,					/* jog absoluto */

	EMCMOT_JOG_ABORT,					/* abortar una articulación núm. o un eje núm. */
	EMCMOT_JOINT_ACTIVATE,				/* hacer articulación activa */
	EMCMOT_JOINT_DEACTIVATE,			/* hacer articulación inactiva */
	EMCMOT_JOINT_ENABLE_AMPLIFIER,		/* habilitar salidas de amplificador */
	EMCMOT_JOINT_DISABLE_AMPLIFIER,		/* deshabilitar salidas de amplificador */
	EMCMOT_JOINT_HOME, 					/* coloca en home una o todas las articulaciones */
	EMCMOT_JOINT_UNHOME,				/* unhome una o todas las articulaciones*/
	EMCMOT_SET_JOINT_POSITION_LIMITS,	/* establece los límites +/- de la posición de la articulación */
	EMCMOT_SET_JOINT_BACKLASH,			/* establece el backlash de la articulación */
	EMCMOT_SET_JOINT_MIN_FERROR,		/* error de seguimiento mínimo, unidades de entrada */
	EMCMOT_SET_JOINT_MAX_FERROR,		/* error de seguimiento máximo, unidades de entrada */
	EMCMOT_SET_JOINT_VEL_LIMIT,			/* establece la velocidad máxima de la articulación */
	EMCMOT_SET_JOINT_ACC_LIMIT,			/* establece la aceleración máxima de la articulación */
	EMCMOT_SET_JOINT_HOMING_PARAMS,		/* establece parámetros home de articulacion */
	EMCMOT_UPDATE_JOINT_HOMING_PARAMS,	/* actualiza algunos parámetros de homing de articulacion */
	EMCMOT_SET_JOINT_MOTOR_OFFSET,		/* establece el offset entre la articulación y el motor */
	EMCMOT_SET_JOINT_COMP,				/* establece un triplete de compensación para una articulación (nominal, adelante, atrás) */

	EMCMOT_SET_AXIS_POSITION_LIMITS,	/* establece los límites +/- de la posición del eje */
	EMCMOT_SET_AXIS_VEL_LIMIT,			/* establece la velocidad máxima del eje */
	EMCMOT_SET_AXIS_ACC_LIMIT,			/* establece la aceleración máxima del eje */
	EMCMOT_SET_AXIS_LOCKING_JOINT,		/* set the axis locking joint */

	EMCMOT_SET_SPINDLE_PARAMS,			/* Un comando para configurar todos los parámetros del husillo */

    } cmd_code_t;

/* Esta enum lista los posibles resultados de un comando */

    typedef enum {
	EMCMOT_COMMAND_OK = 0,             /* comando respetado */
	EMCMOT_COMMAND_UNKNOWN_COMMAND,    /* cmd desconocido */
	EMCMOT_COMMAND_INVALID_COMMAND,    /* cmd invalido ahora */
	EMCMOT_COMMAND_INVALID_PARAMS,     /* parámetros de comando invalidos */
	EMCMOT_COMMAND_BAD_EXEC            /* error al intentar iniciar */
    } cmd_status_t;

/* condiciones de terminación para movimientos en cola */

#define EMCMOT_TERM_COND_STOP 1     /* por parada */
#define EMCMOT_TERM_COND_BLEND 2    /* por mezcla */
#define EMCMOT_TERM_COND_TANGENT 3  /* por tangencia */

/*********************************
       ESTRUCTURA DE COMANDOS
*********************************/

/* Hay una estructura de comandos en la memoria compartida y todos los comandos del código de nivel superior pasan por ella.
*/
    typedef struct emcmot_command_t {
    cmd_code_t command; 		/* código de comando (enumeración) */
    int commandNum; 			/* incrementa esto para el nuevo comando */
    double motor_offset; 		/* desplazamiento desde articulación a posición del motor */
    double maxLimit; 			/* valor pos para el límite de posición, salida */
    double minLimit; 			/* valor negativo para el límite de posición, salida */
    double min_pos_speed; 		/* velocidad mínima positiva del husillo */
    double max_neg_speed; 		/* velocidad negativa máxima del husillo */
    EmcPose pos; 				/* punto final de línea/círculo, o vector teleop */
    PmCartesian center; 		/* centro del círculo */
    PmCartesian normal; 		/* vector normal para el círculo */
    int turn;                   /* vueltas para el círculo o número de articulación para un indexador de bloqueo */
    double vel; 				/* velocidad máxima */
    double ini_maxvel;          /* velocidad máxima permitida por las restricciones de la máquina (el archivo INI) */
    int motion_type;            /* movimiento por desplazamiento, avance, arco o cambio de herramienta */
    double spindlesync;         /* unidades de usuario por revolución del husillo, 0 = sin sincronización */
    double acc;                 /* aceleración máxima */
    double backlash;            /* cantidad de backlash */
    int id;                     /* id para el movimiento */
    int termCond;				/* condición de terminación */
    double tolerance; 			/* tolerancia para desviación de trayectoria en modo CONTINUO */
    int joint;                  /* qué índice de articulación utilizar a continuación */
    int axis;                   /* qué índice de eje utilizar para lo siguiente */
    int spindle; 				/* qué husillo utilizar */
    double scale; 				/* escala de velocidad o escala de velocidad del husillo arg */
    double offset; 				/* argumento de offset de input, output, o home*/
    double home; 				/* posición home de articulación */
    double home_final_vel; 		/* velocidad de la articulación para moverse desde OFFSET a HOME */
    double search_vel; 			/* velocidad de búsqueda de inicio */
    double latch_vel; 			/* velocidad del pestillo de inicio */
    int flags; 	                /* indicadores de configuración de inicio, otros argumentos booleanos */
    int home_sequence; 			/* orden en la secuencia de retorno */
    int volatile_home; 			/* la articulación debe quedar sin hogar cuando obtenemos unhome -2 (generado por la tarea al detenerse, etc.) */
    double minFerror; 			/* error de seguimiento mínimo */
    double maxFerror; 			/* error máximo de seguimiento */
    int wdWait; 				/* ciclo de espera antes de alternar wd */
    int debug;                  /* nivel de depuración, de DEBUG en el archivo INI */
    unsigned char now, out, start, end; 	/* estos están relacionados con AOUT/DOUT sincronizados.
                                            now=ya sea ahora o sincronizado,
                                            out = cuál se establece,
                                            start=valor inicial,
                                            end=valor final */
    unsigned char mode; 		/* se utiliza para activar o desactivar anulaciones, etc. */
    double comp_nominal, comp_forward, comp_reverse; 	 /* triplete de compensación, nominal, adelante, atrás */
    unsigned char probe_type; 	/*  ~1 = error si la operación de sondeo no es exitosa (valor predeterminado de ngc)
                                    |1 = suprimir error, informar en # en su lugar
                                    ~2 = mover hasta que se active la sonda (valor predeterminado de ngc) 
                                    |2 = mover hasta que la sonda se despeje */
    int probe_jog_err_inhibit; 	/* configuración para inhibir activación de sonda durante error jog */
    int probe_home_err_inhibit;	/* configuración para inhibir activación de sonda durante error home */
    EmcPose tool_offset;		/* TLO */
    double orientation; 		/* ángulo para orientar el husillo */
    int state;                  /* estado del husillo */
    char direction; 	 		/* Indicador CANON_DIRECTION para la orientación del husillo */
    double timeout; 			/* de espera para que se complete la orientación del husillo */
    unsigned char wait_for_spindle_at_speed; 	 /* EMCMOT_SPINDLE_ON ahora lleva esto, para el siguiente movimiento de avance */
    int arcBlendOptDepth; 		/* */
    int arcBlendEnable; 		/* */
    int arcBlendFallbackEnable; /* */
    int arcBlendGapCycles; 		/* */
    double arcBlendRampFreq; 	/* */
    double arcBlendTangentKinkRatio; 	/* */
    double maxFeedScale; 		/* */
    double ext_offset_vel; 		/* velocidad para un desplazamiento del eje externo */
    double ext_offset_acc; 		/* aceleración para un desplazamiento del eje externo */
struct state_tag_t tag; 	 	/* */
    } emcmot_command_t;

/*! \todo FIXME - estos bits empaquetados podrían reemplazarse con caracteres. La memoria es barata y sería bueno poder acceder a ellos sin esas feas macros.
*/

/* tipo de bandera motion */
    typedef unsigned short EMCMOT_MOTION_FLAG;

/*
  estructura de bandera de estado de movimiento:

  MSB                             LSB
  v---------------v------------------v
  |   |   |   | T | CE | C | IP | EN |
  ^---------------^------------------^

  donde:

  EN es 1 si los cálculos están habilitados, 0 si no
  IP es 1 si todas las articulaciones están en posición, 0 si no
  C es 1 si está en modo coordinado, 0 si está en modo libre
  CE es 1 si hay error de modo coordinado, 0 si no
  T es 1 si modo teleoperador.
  */

/* bit masks */
#define EMCMOT_MOTION_ENABLE_BIT      0x0001
#define EMCMOT_MOTION_INPOS_BIT       0x0002
#define EMCMOT_MOTION_COORD_BIT       0x0004
#define EMCMOT_MOTION_ERROR_BIT       0x0008
#define EMCMOT_MOTION_TELEOP_BIT      0x0010

/* tipo de flag de articulación */
    typedef unsigned short EMCMOT_JOINT_FLAG;
/*
  Estructura de flag de estado de articulación:

  MSB                                                          LSB
  ----------v-----------------v--------------------v-------------------v
  | AF | FE | AH | HD | H | HS | NHL | PHL | x | x | ER | IP | AC | EN |
  ----------^-----------------^--------------------^-------------------^


  x = sin uso

  donde:

  EN es 1 si amplificador de articulacion habilitado, 0 si no lo está
  AC es 1 si la articulación está activa para los cálculos, 0 si no
  IP es 1 si la articulación está en posición, 0 si no (solo en modo libre)
  ER es 1 si la articulación tiene un error, 0 si no

  PHL es 1 si la articulación está en el límite máximo de hardware, 0 si no
  NHL es 1 si la articulación está en el límite mínimo de hardware, 0 si no

  HS es 1 si se activa el interruptor home de articulacion, 0 si no
  H es 1 si la articulación está haciendo homing, 0 si no
  HD es 1 si la articulación esta siendo puesta homed, 0 si no
  AH es 1 si la articulación está en home, 0 si no

  FE es 1 si la articulación excede el error de seguimiento , 0 si no
  AF es 1 si el amplificador falla, 0 si no

Sugerencia: Dividir esto en un registro de flags de error y uno de estado.
Luego, se puede realizar una prueba simple en cada una de las flags en lugar de probar cada bit... Guardando en una flag global de estado listo y falla por articulación.
  */

/* máscaras de bits */
#define EMCMOT_JOINT_ENABLE_BIT         0x0001
#define EMCMOT_JOINT_ACTIVE_BIT         0x0002
#define EMCMOT_JOINT_INPOS_BIT          0x0004
#define EMCMOT_JOINT_ERROR_BIT          0x0008
#define EMCMOT_JOINT_MAX_HARD_LIMIT_BIT 0x0010
#define EMCMOT_JOINT_MIN_HARD_LIMIT_BIT 0x0020
#define EMCMOT_JOINT_FERROR_BIT         0x0040
#define EMCMOT_JOINT_FAULT_BIT          0x0080


/*! \todo FIXME – los términos “teleop”, “coord” y “free” están mal documentados. Este es mi débil intento de entender exactamente lo que significan.

Según Fred, teleop nunca se utiliza con máquinas herramienta, aunque eso puede no ser cierto para máquinas con cinemática no trivial.

El modo “coord”, o coordinado, significa que todas las articulaciones están sincronizadas y se mueven juntas según lo ordena el código de nivel superior. Es el modo normal durante el mecanizado. En el modo coordinado, se supone que los comandos están en el marco de referencia cartesiano y, si la máquina no es cartesiana, los comandos son traducidos por la cinemática para manejar cada articulación en el espacio de la articulación según sea necesario.

El modo “free” significa que los comandos se interpretan en el espacio articular.
Se utiliza para jogging de articulaciones individuales, aunque no impide mover varias articulaciones a la vez (creo).
El retorno al origen también se realiza en modo libre; de hecho, las máquinas con cinemática no trivial deben ser llevadas al origen antes de poder pasar al modo coord o teleop.

"Teleop" es probablemente lo que necesitas si estás "moviendo" un hexápodo. Los comandos de movimiento implementados por el controlador de movimiento son movimientos articulares, que funcionan en modo libre. Pero si se quiere mover un hexápodo o una máquina similar a lo largo de un eje cartesiano en particular, se necesita operar más de una articulación. Para eso está "teleop".

*/

/* estructuras de compensación */
    typedef struct {
	double nominal;		/* posición nominal (de comando) */
	float fwd_trim;		/* corrección para movimiento hacia adelante */
	float rev_trim;		/* corrección para movimiento inverso */
	float fwd_slope;	/* pendientes entre puntos actual y siguiente */
	float rev_slope;
    } emcmot_comp_entry_t;


#define EMCMOT_COMP_SIZE 256
    typedef struct {
	int entries;                   /* número de entradas en la matriz */
	emcmot_comp_entry_t *entry;    /* entrada actual en la matriz */
	emcmot_comp_entry_t array[EMCMOT_COMP_SIZE+2];
	/* +2 porque la matriz tiene entradas -HUGE_VAL y +HUGE_VAL en los extremos */
    } emcmot_comp_t;

/* estados del controlador motion */

    typedef enum {
	EMCMOT_MOTION_DISABLED = 0,
	EMCMOT_MOTION_FREE,
	EMCMOT_MOTION_TELEOP,
	EMCMOT_MOTION_COORD
    } motion_state_t;

/*! \todo FIXME – ¿que es esta enum?
    typedef enum {
	EMCMOT_ORIENT_NONE = 0,
	EMCMOT_ORIENT_COMPLETE,
	EMCMOT_ORIENT_IN_PROGRESS,
	EMCMOT_ORIENT_FAULTED,
    } orient_state_t;

/* flags  para habilitar....  */

#define SS_ENABLED 0x01     /* escalado del husillo
#define FS_ENABLED 0x02     /* escalado de avance
#define AF_ENABLED 0x04     /* avance adaptativo
#define FH_ENABLED 0x08     /* retención de avance

/* La siguiente estructura contiene todos los datos asociados con una única articulación. Aunque no es necesario que esta estructura esté en la memoria compartida (puede estarlo si se desea por razones de depuración). Las partes de esta estructura que se consideran "estado" y que deben estar disponibles para el espacio del usuario se copian en una estructura mucho más pequeña denominada emcmot_joint_status_t que se encuentra en la memoria compartida.

*/
    typedef struct {

	/* información de configuración - cambia raramente */
	int type;/* 0 = lineal, 1 = rotatorio */
	double max_pos_limit;  /* límite superior soft en la posición de la articulación */
	double min_pos_limit;  /* límite soft inferior en la posición de la articulación */
	double max_jog_limit;  /* los límites de jog cambian cuando no se está en home */
	double min_jog_limit;
	double vel_limit;      /* límite superior de la velocidad de la articulación */
	double acc_limit;      /* límite superior de aceleración de la articulación */
	double min_ferror;     /* límite de error de seguimiento de velocidad cero */
	double max_ferror;     /* límite de error de seguimiento de velocidad máxima */
	double backlash;       /* cantidad de backlash */
	emcmot_comp_t comp;    /* datos de corrección del husillo de avance */

	/* información de estado - cambia periódicamente */
	/* muchos de estos deben estar disponibles para niveles superiores */
	/* se pueden copiar a la estructura de estado o a una matriz de
	estructuras de articulación pueden convertirse en parte del estado */
	EMCMOT_JOINT_FLAG flag;		/* ver arriba para detalles de bits */
	double coarse_pos; 			/* punto de trayectoria, antes de interp */
	double pos_cmd; 			/* posición de la articulación comandada */
	double vel_cmd;				/* velocidad articular comandada */
	double acc_cmd; 			/* aceleración articular comandada */
	double backlash_corr; 		/* corrección por backlash */
	double backlash_filt; 		/* corrección por backlash filtrada */
	double backlash_vel; 		/* variable de velocidad de backlash */
	double motor_pos_cmd; 		/* posición comandada, con compensación */
	double motor_pos_fb; 		/* retroalimentación de posición, con compensación */
	double pos_fb; 				/* retroalimentación de posición, compensación eliminada */
	double ferror; 				/* error de seguimiento */
	double ferror_limit;		/* el límite depende de la velocidad */
	double ferror_high_mark; 	/* error máximo de seguimiento */
	simple_tp_t free_tp; 		/* planificador para movimiento en modo libre */
	int kb_jjog_active; 		/* distinto de cero durante un jog de teclado */
	int wheel_jjog_active; 		/* distinto de cero durante un jog de volante */

	/* información interna: cambia periódicamente, normalmente no accede a ella el espacio de usuario*/
	CUBIC_STRUCT cubic;	/* datos del interpolador cúbico */

	int on_pos_limit;      /* distinto de cero si está en el límite pos */
	int on_neg_limit;      /* distinto de cero si está en el límite neg */

	double motor_offset;   /* diferencia entre la posición interna y la del motor, usada para poner la posición a cero durante el retorno a home */
	int old_jjog_counts;   /* valor anterior, usado para deltas */
	double big_vel;        /* usado para "debouncing" de la velocidad */
    } emcmot_joint_t;

/* La siguente estructura contiene únicamente los datos de “estado” asociados con una articulación. Los datos de “estado” son los datos que se deben informar al espacio de usuario de manera continua. Una matriz de estas estructuras es parte de la estructura de estado principal y se completa con datos copiados de las estructuras emcmot_joint_t en cada período de servo.

   Por ahora, esta estructura contiene más datos de los que realmente se necesita, pero reducirla llevará tiempo (y probablemente deba hacerse de a uno o dos elementos a la vez, con muchas pruebas). Mi objetivo principal en este momento es sacar status de la gran estructura articular.

*/
    typedef struct {
	EMCMOT_JOINT_FLAG flag;	/* ver arriba para detalles de bits */
    bool homed;
    bool homing;

	double pos_cmd;            /* posición de articulación ordenada */
	double pos_fb;             /* retroalimentación de posición, compensación eliminada */
	double vel_cmd;            /* velocidad actual */
	double acc_cmd;            /* aceleración actual */
	double ferror;             /* error de seguimiento */
	double ferror_high_mark;   /* error máximo de seguimiento */

/*! \todo FIXME – los siguientes no son realmente “estados”, pero taskintf.cc espera que estén en la estructura de estado. No sé cómo o si son utilizados por el código del espacio de usuario. Lo ideal sería eliminarlos de aquí, pero cada uno deberá investigarse individualmente.

    emcmot_joint_status_t se usa en:
        src/emc/motion/control.c
        src/emc/motion-logger/motion-logger.c
        src/emc/task/taskintf.cc
*/
	double backlash;	/* cantidad de backlash */
	double max_pos_limit;	/* límite superior soft en la posición de la articulación */
	double min_pos_limit;	/* límite soft inferior en la posición de la articulación */
	double min_ferror;	/* límite de error de seguimiento de velocidad cero */
	double max_ferror;	/* límite de error de seguimiento de velocidad máxima */
    } emcmot_joint_status_t;


    typedef struct {
	double speed;		// velocidad del husillo en RPM
	double scale; 		// valor de anulación del husillo
	double net_scale;   // escala o cero si está inhibido
	double css_factor;
	double xoffset;
	int state;
	int direction;		// 0 stopped, 1 forward, -1 reverse
	int brake;		// 0 liberado, 1 activado
	int locked;             // bloqueo del husillo activado después de orientar
	int orient_fault;       // código de fallo de motion.spindle-orient-fault
	int orient_state;       // orient_state_t
	int spindle_index_enable;  /* conectado a un encoder canonico index-enable */
	double spindleRevs;     /* posición del husillo en revoluciones */
	double spindleSpeedIn;  /* velocidad del husillo en rpm */
	int at_speed;
	int fault; /* fallo del amplificador */
	double max_pos_speed; /* límites de velocidad del husillo */
	double min_pos_speed; /* valores con signo, por lo que max_neg = 0 */
	double max_neg_speed; /* y min_neg = -1e99 indica que no hay límite */
	double min_neg_speed;
	double home_angle;
	double home_search_vel;
	int home_sequence;
	double increment;
    } spindle_status_t;

    typedef struct {
	double teleop_vel_cmd;		/* velocidad del eje comandada */
	double max_pos_limit;	/* límite superior soft en la posición del eje */
	double min_pos_limit;	/* límite soft inferior en la posición del eje */
    } emcmot_axis_status_t;

/*********************************
        ESTRUCTURA DE STATUS
*********************************/

/* Esta es la estructura de estado. Hay una de estas en la memoria compartida y reporta el estado del controlador de movimiento al código de nivel superior en el espacio de usuario. En su mayor parte, esta estructura contiene variables de nivel superior: el contenido de bajo nivel que se hace visible para HAL, resolución de problemas, etc., se realiza mediante el osciloscopio de HAL.
*/

/*! \todo FIXME - esta estructura está dividida en dos partes... en la parte superior están los miembros de la estructura que entiendo y que son necesarios para emc2.
A continuación, se suman otros miembros de la estructura. Todos los que se encuentren más adelante deben evaluarse: o ascienden o desaparecen.
*/

    typedef struct emcmot_status_t {
	unsigned char head;	/* conteo de flags para detección de mutex */
	/* los siguientes tres se actualizan solo ante un nuevo comando */
	cmd_code_t commandEcho;	/* eco del comando de entrada */
	int commandNumEcho;	/* eco del número del comando de entrada */
	cmd_status_t commandStatus;	/* resultado del comando más reciente */
	/* información de configuración; se actualiza al cambiarla un comando */
	double feed_scale;	/* factor de escala de velocidad para todos los movimientos excepto los rápidos */
	double rapid_scale;	/* factor de escala de velocidad para rápidos */
	unsigned char enables_new;	/* flags para FS, SS, etc */
    /* el conjunto anterior es el que está habilitado para nuevos movimientos. El resto se actualiza cada ciclo */
	double net_feed_scale;	/* factor de escala neto para todos los movimientos */
	unsigned char enables_queued;	/* flags para FS, SS, etc */
		/* el conjunto anterior son las habilitaciones vigentes para el
		movimiento actualmente en ejecución  */
	motion_state_t motion_state; /* estado operativo: FREE, COORD, etc. */
	EMCMOT_MOTION_FLAG motionFlag;	/* ver arriba para detalles de bits */
	EmcPose carte_pos_cmd;	/* posición cartesiana ordenad */
	int carte_pos_cmd_ok;	/* distinto de cero si el comando es válido */
	EmcPose carte_pos_fb;	/* posición cartesiana actual */
	int carte_pos_fb_ok;	/* distinto de cero si feedback es válido */
	EmcPose world_home;	/* coordenadas cartesianas de la posición home */
	emcmot_joint_status_t joint_status[EMCMOT_MAX_JOINTS];	/* todos los datos sobre el estado de las articulaciones */
    emcmot_axis_status_t axis_status[EMCMOT_MAX_AXIS];	/* todos los datos de estado del eje*/
    int spindleSync;    /* husillo utilizado para movimientos sincronizados. -1 = ninguno */
    spindle_status_t spindle_status[EMCMOT_MAX_SPINDLES]; /* todos los datos del husillo */


	int on_soft_limit;	/* non-zero if any joint is on soft limit */

	int probeVal;		/* debounced value of probe input */

	int probeTripped;	/* Has the probe signal changed since start
				   of probe command? */
	int probing;		/* Currently looking for a probe signal? */
        unsigned char probe_type;
	EmcPose probedPos;	/* Axis positions stored as soon as possible
				   after last probeTripped */


	int synch_di[EMCMOT_MAX_DIO]; /* inputs to the motion controller, queried by G-code */
	int synch_do[EMCMOT_MAX_DIO]; /* outputs to the motion controller, queried by G-code */
	double analog_input[EMCMOT_MAX_AIO]; /* inputs to the motion controller, queried by G-code */
	double analog_output[EMCMOT_MAX_AIO]; /* outputs to the motion controller, queried by G-code */
	int misc_error[EMCMOT_MAX_MISC_ERROR]; /* Random Error pins*/
	struct state_tag_t tag; /* Current interp state corresponding
				   to motion line */

/*! \todo FIXME - all structure members beyond this point are in limbo */

	/* dynamic status-- changes every cycle */
	unsigned int heartbeat;
	int config_num;		/* incremented whenever configuration
				   changed. */
	int id;			/* id for executing motion */
	int depth;		/* motion queue depth */
	int activeDepth;	/* depth of active blend elements */
	int queueFull;		/* Flag to indicate the tc queue is full */
	int paused;		/* Flag to signal motion paused */
	int overrideLimitMask;	/* non-zero means one or more limits ignored */
				/* 1 << (joint-num*2) = ignore neg limit */
				/* 2 << (joint-num*2) = ignore pos limit */
    int reverse_run;

	/* static status-- only changes upon input commands, e.g., config */
	double vel;		/* scalar max vel */
	double acc;		/* scalar max accel */

	int motionType;
	double distance_to_go;  /* in this move */
	EmcPose dtg;
	double current_vel;
	double requested_vel;

	unsigned int tcqlen;
	EmcPose tool_offset;
	int atspeed_next_feed;  /* at next feed move, wait for spindle to be at speed  */
	unsigned char tail;	/* flag count for mutex detect */
	int external_offsets_applied;
	EmcPose eoffset_pose;
	int numExtraJoints;
    int stepping;
    bool jogging_active;
    } emcmot_status_t;

/*********************************
        ESTRUCTURA CONFIG
*********************************/

/* This is the config structure.  This is currently in shared memory,
   but I have no idea why... there are commands to set most of the
   items in this structure.  It seems we should either put the struct
   in private memory and manipulate it with commands, or we should
   put it in shared memory and manipulate it directly - not both.
   The structure contains static or rarely changed information that
   describes the machine configuration.

   later: I think I get it now - the struct is in shared memory so
   user space can read the config at any time, but commands are used
   to change the config so they only take effect when the realtime
   code processes the command.
*/

/*! \todo FIXME - esta estructura está dividida en dos partes... en la parte superior están los miembros de la estructura que entiendo y que son necesarios para emc2.
A continuación, se suman otros miembros de la estructura. Todos los que se encuentren más adelante deben evaluarse: o ascienden o desaparecen.
*/
    typedef struct emcmot_config_t {
	unsigned char head;	/* flag count for mutex detect */

	int config_num;		/* Incremented everytime configuration
				   changed, should match status.config_num */
	int numJoints;		/* The number of total joints in the system (which
				   must be between 1 and EMCMOT_MAX_JOINTS,
				   inclusive). includes extra joints*/
	int numExtraJoints;		/* The number of extra joints in the system (which
				   must be between 1 and EMCMOT_MAX_EXTRAJOINTS,
				   inclusive). */
	int numSpindles; /* The number of spindles, 1 to EMCMOT_MAX_SPINDLES */

	KINEMATICS_TYPE kinType;

        int numDIO;             /* userdefined number of digital IO. default is 4. (EMCMOT_MAX_DIO=64),
                                   but can be altered at motmod insmod time */

        int numAIO;             /* userdefined number of analog IO. default is 4. (EMCMOT_MAX_AIO=16),
                                   but can be altered at motmod insmod time */

        int numMiscError;     /* userdefined number of Misc Errors. default is 0.
                                  but can be altered at motmod insmod time */

/*! \todo FIXME - all structure members beyond this point are in limbo */

	double trajCycleTime;	/* the rate at which the trajectory loop
				   runs.... (maybe) */
	double servoCycleTime;	/* the rate of the servo loop - Not the same
				   as the traj time */

	int interpolationRate;	/* grep control.c for an explanation....
				   approx line 50 */

	double limitVel;	/* scalar upper limit on vel */
	int debug;		/* copy of DEBUG, from INI file */
	unsigned char tail;	/* flag count for mutex detect */
        int arcBlendOptDepth;
        int arcBlendEnable;
        int arcBlendFallbackEnable;
        int arcBlendGapCycles;
        double arcBlendRampFreq;
        double arcBlendTangentKinkRatio;
        double maxFeedScale;
        int inhibit_probe_jog_error;
        int inhibit_probe_home_error;
    } emcmot_config_t;

/* estructura de error - Un buffer de anillo utilizado para pasar cadenas printf formateadas al espacio de usuario */
    typedef struct emcmot_error_t {
	unsigned char head;	/* flag count for mutex detect */
	char error[EMCMOT_ERROR_NUM][EMCMOT_ERROR_LEN];
	int start;		/* index of oldest error */
	int end;		/* index of newest error */
	int num;		/* number of items */
	unsigned char tail;	/* flag count for mutex detect */
    } emcmot_error_t;


typedef struct emcmot_internal_t {
    unsigned char head; /* flag count for mutex detect */
    unsigned char tail; /* flag count for mutex detect */
    int split;          /* number of split command reads */
    int enabling;       /* starts up disabled */
    int coordinating;   /* starts up in free mode */
    int teleoperating;  /* starts up in free mode */
    int overriding;     /* non-zero means we've initiated an joint
                           move while overriding limits */
    TP_STRUCT coord_tp; /* coordinated mode planner */
    int idForStep;      /* status id while stepping */
    } emcmot_internal_t;

/* error ring buffer access functions */
    extern int emcmotErrorInit(emcmot_error_t * errlog);
    extern int emcmotErrorPut(emcmot_error_t * errlog, const char *error);
    extern int emcmotErrorPutfv(emcmot_error_t * errlog, const char *fmt, va_list ap);
    extern int emcmotErrorPutf(emcmot_error_t * errlog, const char *fmt, ...);
    extern int emcmotErrorGet(emcmot_error_t * errlog, char *error);

#define GET_JOINT_ACTIVE_FLAG(joint) ((joint)->flag & EMCMOT_JOINT_ACTIVE_BIT ? 1 : 0)
#define GET_JOINT_INPOS_FLAG(joint) ((joint)->flag & EMCMOT_JOINT_INPOS_BIT ? 1 : 0)

#ifdef __cplusplus
}
#endif
#endif    /* MOTION_H */
