/* AUTORIGHTS
Copyright (C) 2007 Princeton University
      
This file is part of Ferret Toolkit.

Ferret Toolkit is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <cass.h>
#include <cass_timer.h>
#include <../image/image.h>
#include "tpool.h"
#include "queue.h"


#include "grppi/grppi.h"

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif


#include <sys/time.h>
static double time_begin, time_end;

#define DEFAULT_DEPTH	25
#define MAXR	100
#define IMAGE_DIM	14

char *db_dir = NULL;
const char *table_name = NULL;
const char *query_dir = NULL;
const char *output_path = NULL;

FILE *fout;

DIR *pd = NULL;

char * outputFile;

int NTHREAD_LOAD = 1;
int NTHREAD_SEG	= 1;
int NTHREAD_EXTRACT = 1;
int NTHREAD_VEC	= 1;
int NTHREAD_RANK = 1;
int NTHREAD_OUT	= 1;
int DEPTH = DEFAULT_DEPTH;
int TOTAL_THREADS;

int top_K = 10;

char *extra_params = "-L 8 - T 20";

cass_env_t *env;
cass_table_t *table;
cass_table_t *query_table;

int vec_dist_id = 0;
int vecset_dist_id = 0;

struct load_data
{
	int width, height;
	char *name;
	unsigned char *HSV, *RGB;
};

struct queue q_load_seg;

struct seg_data
{
	int width, height, nrgn;
	char *name;
	unsigned char *mask;
	unsigned char *HSV;
};

struct queue q_seg_extract;

struct extract_data
{
	cass_dataset_t ds;
	char *name;
};

struct queue q_extract_vec;

struct vec_query_data
{
	char *name;
	cass_dataset_t *ds;
	cass_result_t result;
};

struct queue q_vec_rank;

struct rank_data
{
	char *name;
	cass_dataset_t *ds;
	cass_result_t result;
};

struct queue q_rank_out;


/* ------- The Helper Functions ------- */
int cnt_enqueue;
int cnt_dequeue;
char path[BUFSIZ];




/* ------ The Stages ------ */
std::experimental::optional<load_data *>t_load()
{
	struct dirent *ent = NULL;

	int i = 0;
	while(ent = readdir(pd))
	{

			
		if (ent == NULL) break;

		if (ent->d_name[0] == '.')
		{
			if (ent->d_name[1] == 0) continue;
			else if (ent->d_name[1] == '.')
			{
				if (ent->d_name[2] == 0) continue;
			}
		}



		int r;
		struct load_data *data;

		char * strp = (char*)malloc(80*sizeof(char));
		strcpy(strp, query_dir);


		data = (struct load_data *)malloc(sizeof(struct load_data));
		assert(data != NULL);

		data->name = strdup(ent->d_name);

		
		strcat(strp, "/");
		strcat(strp, ent->d_name);

		r = image_read_rgb_hsv(strp, &data->width, &data->height, &data->RGB, &data->HSV);
		assert(r == 0);
		cnt_enqueue++;
	

		if (data==NULL) perror("Wow,es null\n");
		return data;
	}

	if (pd!=NULL) closedir(pd);
	return {};
}

seg_data *t_seg (load_data *load)
{
	struct seg_data *seg;

	while(1)
	{

		assert(load != NULL);
		seg = (struct seg_data *)calloc(1, sizeof(struct seg_data));

		seg->name = load->name;

		seg->width = load->width;
		seg->height = load->height;
		seg->HSV = load->HSV;
		image_segment((void**)&seg->mask, &seg->nrgn, load->RGB, load->width, load->height);

		free(load->RGB);
		free(load);

		return seg;
	}

	return NULL;

}

extract_data *t_extract (seg_data *seg)
{
	struct extract_data *extract;

	while (1)
	{
		assert(seg != NULL);
		extract = (struct extract_data *)calloc(1, sizeof(struct extract_data));

		extract->name = seg->name;

		image_extract_helper(seg->HSV, seg->mask, seg->width, seg->height, seg->nrgn, &extract->ds);

		free(seg->mask);
		free(seg->HSV);
		free(seg);

		return extract;
	}

	return NULL;
}

vec_query_data *t_vec (extract_data* extract)
{
	struct vec_query_data *vec;
	cass_query_t query;
	while(1)
	{
		assert(extract != NULL);
		vec = (struct vec_query_data *)calloc(1, sizeof(struct vec_query_data));
		vec->name = extract->name;

		memset(&query, 0, sizeof query);
		query.flags = CASS_RESULT_LISTS | CASS_RESULT_USERMEM;

		vec->ds = query.dataset = &extract->ds;
		query.vecset_id = 0;

		query.vec_dist_id = vec_dist_id;

		query.vecset_dist_id = vecset_dist_id;

		query.topk = 2*top_K;

		query.extra_params = extra_params;

	    	cass_result_alloc_list(&vec->result, vec->ds->vecset[0].num_regions, query.topk);

	//	cass_result_alloc_list(&result_m, 0, T1);
	//	cass_table_query(table, &query, &vec->result);
		cass_table_query(table, &query, &vec->result);

		return vec;
	}
	return NULL;
}

rank_data *t_rank (vec_query_data * vec)
{
	struct rank_data *rank;
	cass_result_t *candidate;
	cass_query_t query;
	while (1)
	{

		
		assert(vec != NULL);

		rank = (struct rank_data *)calloc(1, sizeof(struct rank_data));
		rank->name = vec->name;

		query.flags = CASS_RESULT_LIST | CASS_RESULT_USERMEM | CASS_RESULT_SORT;
		query.dataset = vec->ds;
		query.vecset_id = 0;

		query.vec_dist_id = vec_dist_id;

		query.vecset_dist_id = vecset_dist_id;

		query.topk = top_K;

		query.extra_params = NULL;

		candidate = cass_result_merge_lists(&vec->result, (cass_dataset_t *)query_table->__private, 0);
		query.candidate = candidate;


		cass_result_alloc_list(&rank->result, 0, top_K);
		cass_table_query(query_table, &query, &rank->result);

		cass_result_free(&vec->result);
		cass_result_free(candidate);
		free(candidate);
		cass_dataset_release(vec->ds);
		free(vec->ds);
		free(vec);
		return rank;
	}

	return NULL;
}

void *t_out (rank_data *rank)
{
	
		assert(rank != NULL);

		fprintf(fout, "%s", rank->name);

		ARRAY_BEGIN_FOREACH(rank->result.u.list, cass_list_entry_t p)
		{
			char *obj = NULL;
			if (p.dist == HUGE) continue;
			cass_map_id_to_dataobj(query_table->map, p.id, &obj);
			assert(obj != NULL);
			fprintf(fout, "\t%s:%g", obj, p.dist);
		} ARRAY_END_FOREACH;

		fprintf(fout, "\n");

		cass_result_free(&rank->result);
		free(rank->name);
		free(rank);

		cnt_dequeue++;
		
		//fprintf(stderr, "(%d,%d)\n", cnt_enqueue, cnt_dequeue);
	

	return NULL;
}

int main (int argc, char *argv[])
{
	stimer_t tmr;


	int ret, i;

#ifdef PARSEC_VERSION
#define __PARSEC_STRING(x) #x
#define __PARSEC_XSTRING(x) __PARSEC_STRING(x)
        printf("PARSEC Benchmark Suite Version "__PARSEC_XSTRING(PARSEC_VERSION)"\n");
        fflush(NULL);
#else
        printf("PARSEC Benchmark Suite\n");
        fflush(NULL);
#endif //PARSEC_VERSION
#ifdef ENABLE_PARSEC_HOOKS
	__parsec_bench_begin(__parsec_ferret);
#endif

	if (argc < 8)
	{
		printf("%s <database> <table> <query dir> <top K> <depth> <n> <out>\n", argv[0]); 
		return 0;
	}

	db_dir = argv[1];
	table_name = argv[2];
	query_dir = argv[3];
	top_K = atoi(argv[4]);

	DEPTH = atoi(argv[5]);
	NTHREAD_SEG = 4;
	NTHREAD_EXTRACT = 4;
	NTHREAD_VEC = 4;
	NTHREAD_RANK = atoi(argv[6]);

	TOTAL_THREADS = NTHREAD_RANK+NTHREAD_VEC+NTHREAD_EXTRACT+NTHREAD_SEG+NTHREAD_OUT+NTHREAD_LOAD;

	output_path = argv[7];

	outputFile = argv[8];

	fout = fopen(output_path, "w");
	assert(fout != NULL);


	cass_init();

	ret = cass_env_open(&env, db_dir, 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }

	vec_dist_id = cass_reg_lookup(&env->vec_dist, "L2_float");
	assert(vec_dist_id >= 0);

	vecset_dist_id = cass_reg_lookup(&env->vecset_dist, "emd");
	assert(vecset_dist_id >= 0);

	i = cass_reg_lookup(&env->table, table_name);


	table = query_table = (cass_table_t*)cass_reg_get((cass_reg_t*)&env->table, i);

	i = table->parent_id;

	if (i >= 0)
	{
		query_table = (cass_table_t*)cass_reg_get((cass_reg_t*)&env->table, i);
	}

	if (query_table != table) cass_table_load(query_table);
	
	cass_map_load(query_table->map);

	cass_table_load(table);

	image_init(argv[0]);

	

	cnt_enqueue = cnt_dequeue = 0;

	
	pd = opendir(query_dir);
	if (pd == NULL) perror("Error lectura");

    stimer_tick(&tmr);
#ifdef ENABLE_PARSEC_HOOKS
	__parsec_roi_begin();
#endif

  struct timeval t;
  gettimeofday(&t,NULL);
  time_begin = (double)t.tv_sec+(double)t.tv_usec*1e-6;

  grppi::parallel_execution_omp ex = grppi::parallel_execution_omp(TOTAL_THREADS);
  ex.set_queue_attributes(1000, grppi::queue_mode::lockfree);
  ex.disable_ordering();
  grppi::pipeline(ex,
  [&pd]()->std::experimental::optional<load_data *>{return t_load();},
  
  grppi::farm(NTHREAD_SEG,
  [](load_data * v_in){return t_seg(v_in);}),
  grppi::farm(NTHREAD_EXTRACT,
  [](seg_data * v_in){return t_extract(v_in);}),
  grppi::farm(NTHREAD_VEC,
  [](extract_data * v_in){return t_vec(v_in);}),
  grppi::farm(NTHREAD_RANK,
  [](vec_query_data* v_in){return t_rank(v_in);}),

  [](rank_data * v_in){return t_out(v_in);}

  );
	

  gettimeofday(&t,NULL);
  time_end = (double)t.tv_sec+(double)t.tv_usec*1e-6;

  FILE *file = fopen(outputFile, "a");
  if(file == NULL) {
    printf("ERROR: Unable to open file `%s'.\n", outputFile);
    exit(1);
  }
  int rv = fprintf(file, "%f\n", time_end-time_begin);
  if(rv < 0) {
    printf("ERROR: Unable to write to file `%s'.\n", outputFile);
    fclose(file);
  }
  fclose(file);
#ifdef ENABLE_PARSEC_HOOKS
	__parsec_roi_end();
#endif

	
	stimer_tuck(&tmr, "QUERY TIME");

	ret = cass_env_close(env, 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }

	cass_cleanup();

	image_cleanup();

	fclose(fout);

#ifdef ENABLE_PARSEC_HOOKS
	__parsec_bench_end();
#endif
	return 0;
}

