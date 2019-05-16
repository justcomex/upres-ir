#define UNICODE							// устанавливаем оптимальную кодировку,
										// так как без директивы будет использоватьс€ ANSI 

#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#define PROCNAME_LENGTH  64
#define MANUFACTURER_LEN 13

#define REGS_COUNT 4
#define DESC_COUNT 9
#define BYTE_COUNT 4

typedef struct Descriptor {
	unsigned int value;
	unsigned int size;
} Descriptor;

static void mycpuid(int regs[REGS_COUNT], int func) {
	int ieax, iebx, iecx, iedx;
	_asm {
		mov eax, func	// установка функции команды cpuid
		cpuid			// вызов инструкции cpuid
						// запись результатов выполнени€ команды cpuid
						mov ieax, eax
						mov iebx, ebx
						mov iecx, ecx
						mov iedx, edx
	}
	// запись результатов в массив
	regs[0] = ieax;
	regs[1] = iebx;
	regs[2] = iecx;
	regs[3] = iedx;
}

static int GetL1dCache() {
	// поиск L1d- ЁЎа, если byte == 0xFF
	int ieax, iebx, iecx;
	int func = 4;
	
	for (int param = 0; param < 4; param++) {
		_asm {
			mov eax, func
			mov ecx, param
			cpuid
			mov ieax, eax
			mov iebx, ebx
			mov iecx, ecx
		}

		if ((ieax & 0xFF) == 0x21) {					// EAX[7:0] 		| тип  ЁЎа пам€ти и уровень  ЁЎа
														// 0x21 == 001 00001|  ЁЎ данных и  ЁЎ первого уровн€
			//int threads = ((ieax >> 26) & 0x1F) + 1;	// EAX[31:26]  | неактуально дл€ линеек процессоров Intel c 2010
			//int threads = ((ieax >> 14) & 0x1FF) + 1;	// EAX[25:14]  | неактуально дл€ линеек процессоров Intel c 2010
			int ways = ((iebx >> 22) & 0x3FF) + 1;		// EBX[31:22]  | путь ассоциативности
			int partitions = ((iebx >> 12) & 0x3FF) + 1;// EBX[21:12]  | физическое сегментирование строки
			int linesize = (iebx & 0xFFF) + 1;			// EBX[11:0]   | длина строки 
			int sets = iecx + 1;						// ECX[31:0]   | количество наборов

			//return threads * ways * partitions * linesize * sets / 1024;	| попытка получить общий размер L1d- ЁЎа
			return ways * partitions * linesize * sets / 1024;	// размер L1d- ЁЎа на €дро
		}
	}

	return 0;
}

_declspec(dllexport) int Information(char* pc_name, int* cache_size) {
	TCHAR* buf = (TCHAR*)malloc(sizeof(TCHAR) * PROCNAME_LENGTH);		// выделение пам€ти дл€ буфера
	DWORD buf_size = sizeof(TCHAR) * PROCNAME_LENGTH;					// размер буфера
	GetComputerName((LPWSTR)buf, &buf_size);							// получить им€ ѕ 
	wsprintf((LPWSTR)pc_name, (LPWSTR)buf);								// вывод имени ѕ  в строку, котора€ €вл€етс€ выходным параметром

	char manufacturer[MANUFACTURER_LEN];								// производитель процессора
	manufacturer[MANUFACTURER_LEN - 1] = '\0';
	int CPUInfo[REGS_COUNT];
	mycpuid(CPUInfo, 0);												// при EAX = 0 процессор возвращает 
																		// идентификатор производител€ процессора 
	memcpy(manufacturer,     CPUInfo + 1, sizeof(int));
	memcpy(manufacturer + 4, CPUInfo + 3, sizeof(int));
	memcpy(manufacturer + 8, CPUInfo + 2, sizeof(int));

	if (!strcmp(manufacturer, "GenuineIntel")) {		// если процессор от произодител€ Intel
		Descriptor desc[DESC_COUNT] = { {0x0A,  8}, {0x0C, 16}, {0x0D, 16}, {0x0E, 24},
										{0x2C, 32}, {0x60, 16}, {0x66,  8}, {0x67, 16},
										{0x68, 32} };

		mycpuid(CPUInfo, 0x02);							// получени€ дескрипторов, описывающих кэш-пам€ть и TLB-буфер
		unsigned int num_calls = CPUInfo[0] & 0xFF;		// получение количества выполнений CPUID при EAX = 2

		unsigned int i = 0;
		for (; i < num_calls; i++) {					// получение полной информации о  ЁЎ-пам€ти
			mycpuid(CPUInfo, 0x02);						// получение дескрипторов, описывающих кэш-пам€ть и TLB-буфер
			num_calls = CPUInfo[0] & 0xFF;				// получение количества выполнений CPUID при EAX = 2
		}
		CPUInfo[0] = CPUInfo[0] & ~0xFF;				// игнорирование информации в младшем байте EAX
														// 0xFF == 11111111
		unsigned int L1dSize;							// размер L1d- ЁЎа
		for (int iregs = 0; iregs < REGS_COUNT; iregs++) {
			if (!(CPUInfo[iregs] & 0x80000000)) {		// если информаци€ в регистре получена верно
				for (int bytes = 0; bytes < BYTE_COUNT; bytes++) {
					short byte = (CPUInfo[i] >> bytes * 8) & 0xFF;

					for (int k = 0; k < DESC_COUNT; k++)
						if (byte == desc[k].value) {
							L1dSize = desc[k].size;
							*cache_size = L1dSize;

							return true;
						}
						else
							if (byte == 0xFF) {			// поиск информации с помощью cpuid при func = 4
								L1dSize = GetL1dCache();// получить размер L1d- ЁЎа
								*cache_size = L1dSize;	// вывод размера L1d- ЁЎа в выходной параметр

								return true;
							}
				}
			}
		}
	}

	return false;
}
