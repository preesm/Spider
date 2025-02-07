
#include <spider.h>
#include <stdio.h>
#include "tests.h"
#include "top_hclm.h"

void testOptims(Stack* pisdfStack, Archi* archi){
	ExecutionStat stat;
	FILE* fres;

	try{
		/* no Optims no Ext */
		fres = fopen("noOptims_noExt_M.csv","w+");
		fprintf(fres,"N,M,nSRActors,shMem,schedTime,execTime,optimTime,mappingTime\n");
		spider_setGraphOptim(false);

		/* Sweep M */
		for(int iter=1; iter<=10; iter++){
			printf("N=%d\n", iter);
			char ganttPath[30];
			sprintf(ganttPath, "ederc_nvar_%d.pgantt", iter);
			char srdagPath[30];
			sprintf(srdagPath, "ederc_nvar_%d.gv", iter);

			pisdfStack->freeAll();

			PiSDFGraph *topPisdf = init_top_hclm(
					archi, pisdfStack,
					/*MNext*/	0,
					/*MStart*/	10,
					/*NMax*/ 	20,
					/*NVal*/ 	iter,
					/*NbS*/		4000);

			Platform::get()->rstTime();
			spider_launch(archi, topPisdf);

			spider_printGantt(archi, spider_getLastSRDAG(), ganttPath, "latex.tex", &stat);
//			spider_getLastSRDAG()->print(srdagPath);

			fprintf(fres,
					"%d,%d,%d,%d,%ld,%ld,%ld,%ld\n",
					iter, 10,
					stat.nSRDAGActor,
					stat.memoryUsed,
					stat.schedTime,
					stat.execTime,
					stat.optimTime,
					stat.mappingTime
					);

			free_top_hclm(topPisdf, pisdfStack);
		}

		fclose(fres);
		fres = fopen("noOptims_noExt_N.csv","w+");
		fprintf(fres,"N,M,nSRActors,shMem,schedTime,execTime,optimTime,mappingTime\n");

		/* Sweep N */
		for(int iter=1; iter<=10; iter++){
			printf("N=%d\n", iter);
			char ganttPath[30];
			sprintf(ganttPath, "ederc_nvar_%d.pgantt", iter);
			char srdagPath[30];
			sprintf(srdagPath, "ederc_nvar_%d.gv", iter);

			pisdfStack->freeAll();

			PiSDFGraph *topPisdf = init_top_hclm(
					archi, pisdfStack,
					/*MNext*/	0,
					/*MStart*/	iter,
					/*NMax*/ 	20,
					/*NVal*/ 	10,
					/*NbS*/		4000);

			Platform::get()->rstTime();
			spider_launch(archi, topPisdf);

			spider_printGantt(archi, spider_getLastSRDAG(), ganttPath, "latex.tex", &stat);
//			spider_getLastSRDAG()->print(srdagPath);

			fprintf(fres,
					"%d,%d,%d,%d,%ld,%ld,%ld,%ld\n",
					10, iter,
					stat.nSRDAGActor,
					stat.memoryUsed,
					stat.schedTime,
					stat.execTime,
					stat.optimTime,
					stat.mappingTime
					);

			free_top_hclm(topPisdf, pisdfStack);
		}
		fclose(fres);


		spider_setGraphOptim(true);
		fres = fopen("optims_noExt_N.csv","w+");
		fprintf(fres,"N,M,nSRActors,shMem,schedTime,execTime,optimTime,mappingTime\n");
		for(int iter=1; iter<=10; iter++){
			printf("N=%d\n", iter);
			char ganttPath[30];
			sprintf(ganttPath, "ederc_nvar_%d.pgantt", iter);
			char srdagPath[30];
			sprintf(srdagPath, "ederc_nvar_%d.gv", iter);

			pisdfStack->freeAll();

			PiSDFGraph *topPisdf = init_top_hclm(
					archi, pisdfStack,
					/*MNext*/	0,
					/*MStart*/	10,
					/*NMax*/ 	20,
					/*NVal*/ 	iter,
					/*NbS*/		4000);

			Platform::get()->rstTime();
			spider_launch(archi, topPisdf);

			spider_printGantt(archi, spider_getLastSRDAG(), ganttPath, "latex.tex", &stat);
//			spider_getLastSRDAG()->print(srdagPath);

			fprintf(fres,
					"%d,%d,%d,%d,%ld,%ld,%ld,%ld\n",
					iter, 10,
					stat.nSRDAGActor,
					stat.memoryUsed,
					stat.schedTime,
					stat.execTime,
					stat.optimTime,
					stat.mappingTime
					);

			free_top_hclm(topPisdf, pisdfStack);
		}

		fclose(fres);
		fres = fopen("optims_noExt_M.csv","w+");
		fprintf(fres,"N,M,nSRActors,shMem,schedTime,execTime,optimTime,mappingTime\n");

		for(int iter=1; iter<=10; iter++){
			printf("N=%d\n", iter);
			char ganttPath[30];
			sprintf(ganttPath, "ederc_nvar_%d.pgantt", iter);
			char srdagPath[30];
			sprintf(srdagPath, "ederc_nvar_%d.gv", iter);

			pisdfStack->freeAll();

			PiSDFGraph *topPisdf = init_top_hclm(
					archi, pisdfStack,
					/*MNext*/	0,
					/*MStart*/	iter,
					/*NMax*/ 	20,
					/*NVal*/ 	10,
					/*NbS*/		4000);

			Platform::get()->rstTime();
			spider_launch(archi, topPisdf);

			spider_printGantt(archi, spider_getLastSRDAG(), ganttPath, "latex.tex", &stat);
//			spider_getLastSRDAG()->print(srdagPath);

			fprintf(fres,
					"%d,%d,%d,%d,%ld,%ld,%ld,%ld\n",
					10, iter,
					stat.nSRDAGActor,
					stat.memoryUsed,
					stat.schedTime,
					stat.execTime,
					stat.optimTime,
					stat.mappingTime
					);

			free_top_hclm(topPisdf, pisdfStack);
		}
		fclose(fres);
	}catch(const char* s){
		printf("Exception : %s\n", s);
	}
}

