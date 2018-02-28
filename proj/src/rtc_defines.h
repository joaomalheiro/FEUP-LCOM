#ifndef __RTC_DEFINES_H
#define __RTC_DEFINES_H

#define RTC_ADDRESS_REG       0x70
#define RTC_DATA_REG          0x71
#define RTC_IRQ               8

/* RTC Data "Addresses" */
#define RTC_SECOND            0
#define RTC_MINUTE            2
#define RTC_HOUR              4
#define RTC_DAY               7
#define RTC_MONTH             8
#define RTC_YEAR              9

/* RTC Register "Addresses" */
#define RTC_REG_A            10
#define RTC_REG_B            11
#define RTC_REG_C            12
#define RTC_REG_D            13

#define RTC_REG_A_UIP        BIT(7) /* This bit is enabled when an update is in progress, data unsafe to read */

#define RTC_REG_B_SET        BIT(7) /* If set, inhibits updates of time/date registers */
#define RTC_REG_B_PIE        BIT(6) /* For enabling periodic interrupts */
#define RTC_REG_B_AIE        BIT(5) /* For enabling alarm interrupts */
#define RTC_REG_B_UIE        BIT(4) /* For enabling update interrupts */
#define RTC_REG_B_DM         BIT(2) /* This bit is enabled when register data is binary, disabled when is BCD */

#define RTC_REG_C_PF         BIT(6) /* Periodic interrupt pending */
#define RTC_REG_C_AF         BIT(5) /* Alarm interrupt pending */
#define RTC_REG_C_UE         BIT(4) /* Update interrupt pending */

#endif /* __RTC_DEFINES_H */
