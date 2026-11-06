// Real async worker queue with controlled compiler callbacks. No GPU or game.
#include <stddef.h>
#include <stdint.h>
#define VK_PLATFORM_H_
#define VKAPI_ATTR
#define VKAPI_CALL
#define VKAPI_PTR
#include <vulkan/vulkan_core.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
static double kl_pipe_ms(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1000.+t.tv_nsec/1e6;}
static int kl_env_on(const char *n,int d){const char*v=getenv(n);return v?strcmp(v,"0")!=0:d;}
#define VKI(...) fprintf(stderr,__VA_ARGS__)
typedef struct{VkDevice dev;} fake_device;
static fake_device device;
static fake_device *g_devs[]={&device};
static _Thread_local int replay_thread;
#define KVA_REPLAY_THREAD() replay_thread
static pthread_mutex_t mu=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv=PTHREAD_COND_INITIALIZER;
static int running_game,running_prep,max_game,max_prep,gate,entered,finished,destroyed,cache_live,deferred_destroyed;
static VkResult fake_create(VkDevice d,VkPipelineCache c,uint32_t n,const VkGraphicsPipelineCreateInfo *ci,const VkAllocationCallbacks*a,VkPipeline*out){
    (void)d;(void)c;(void)a;assert(n==1);
    int prep=ci->subpass>=1000;
    pthread_mutex_lock(&mu);
    int *r=prep?&running_prep:&running_game,*max=prep?&max_prep:&max_game;
    ++*r;if(*r>*max)*max=*r;entered++;pthread_cond_broadcast(&cv);
    while(!gate)pthread_cond_wait(&cv,&mu);
    --*r;finished++;pthread_cond_broadcast(&cv);pthread_mutex_unlock(&mu);
    if(ci->subpass%5==0){*out=VK_NULL_HANDLE;return VK_ERROR_OUT_OF_DEVICE_MEMORY;}
    uint64_t *p=malloc(sizeof *p);assert(p);*p=ci->subpass;*out=(VkPipeline)p;return VK_SUCCESS;
}
static void fake_destroy(VkDevice d,VkPipeline p,const VkAllocationCallbacks*a){(void)d;(void)a;free(p);pthread_mutex_lock(&mu);destroyed++;pthread_mutex_unlock(&mu);}
static VkResult fake_cache(VkDevice d,const VkPipelineCacheCreateInfo*c,const VkAllocationCallbacks*a,VkPipelineCache*out){(void)d;(void)c;(void)a;*out=(VkPipelineCache)malloc(8);assert(*out);pthread_mutex_lock(&mu);cache_live++;pthread_mutex_unlock(&mu);return VK_SUCCESS;}
static void fake_destroy_cache(VkDevice d,VkPipelineCache c,const VkAllocationCallbacks*a){(void)d;(void)a;free(c);pthread_mutex_lock(&mu);cache_live--;pthread_mutex_unlock(&mu);}
static VkResult fake_merge(VkDevice d,VkPipelineCache c,uint32_t n,const VkPipelineCache*s){(void)d;(void)c;(void)n;(void)s;return VK_SUCCESS;}
static VkResult fake_cache_data(VkDevice d,VkPipelineCache c,size_t*n,void*p){(void)d;(void)c;if(p)memset(p,0,*n);*n=0;return VK_SUCCESS;}
static void fake_destroy_module(VkDevice d,VkShaderModule s,const VkAllocationCallbacks*a){(void)d;(void)s;(void)a;pthread_mutex_lock(&mu);deferred_destroyed++;pthread_mutex_unlock(&mu);}
static PFN_vkVoidFunction fake_gdpa(VkDevice d,const char*n){(void)d;
#define FN(k,v) if(!strcmp(n,k))return (PFN_vkVoidFunction)v
    FN("vkDestroyPipeline",fake_destroy);FN("vkCreatePipelineCache",fake_cache);FN("vkDestroyPipelineCache",fake_destroy_cache);FN("vkMergePipelineCaches",fake_merge);FN("vkGetPipelineCacheData",fake_cache_data);FN("vkDestroyShaderModule",fake_destroy_module);return NULL;
#undef FN
}
static PFN_vkGetDeviceProcAddr real_gdpa=fake_gdpa;
#include "../build/Klepton/runtime/gfx/kl_vk_async.inc"
static struct timespec deadline(void){struct timespec t;clock_gettime(CLOCK_REALTIME,&t);t.tv_sec+=5;return t;}
static void wait_entered(int want){struct timespec end=deadline();pthread_mutex_lock(&mu);while(entered<want)assert(!pthread_cond_timedwait(&cv,&mu,&end));pthread_mutex_unlock(&mu);}
static void drain(void){struct timespec end=deadline();pthread_mutex_lock(&g_kva_mu);while(g_kva_pending||g_kva_finishing)assert(!pthread_cond_timedwait(&g_kva_ready,&g_kva_mu,&end));assert(g_kva_game_compiling==0);pthread_mutex_unlock(&g_kva_mu);}
static void round_trip(int prep,int count,int deferred){
    VkPipeline handles[32];assert(count<=32);int expected_success=0;
    pthread_mutex_lock(&mu);gate=entered=finished=0;int before=destroyed;pthread_mutex_unlock(&mu);
    VkPipelineShaderStageCreateInfo stage={.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_VERTEX_BIT,.module=(VkShaderModule)1,.pName="main"};
    VkGraphicsPipelineCreateInfo ci={.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,.stageCount=1,.pStages=&stage};
    replay_thread=prep;
    for(int i=0;i<count;i++){ci.subpass=(prep?1000:0)+i+1;expected_success+=ci.subpass%5!=0;assert(kva_create(device.dev,VK_NULL_HANDLE,1,&ci,&handles[i],fake_create));}
    replay_thread=0;
    wait_entered(prep?8:3);usleep(80000);
    pthread_mutex_lock(&mu);assert(entered==(prep?8:3));assert(!running_game||running_game<=3);if(prep)assert(running_prep==8);pthread_mutex_unlock(&mu);
    if(deferred){for(int i=0;i<count;i++)assert(kva_destroy_pipeline(device.dev,(uint64_t)handles[i],fake_destroy));assert(kva_defer_destroy(0,device.dev,1));}
    pthread_mutex_lock(&mu);gate=1;pthread_cond_broadcast(&cv);pthread_mutex_unlock(&mu);drain();
    if(!deferred)for(int i=0;i<count;i++)assert(kva_destroy_pipeline(device.dev,(uint64_t)handles[i],fake_destroy));
    pthread_mutex_lock(&mu);assert(finished==count);assert(destroyed-before==expected_success);assert(!cache_live);assert(!running_game&&!running_prep);pthread_mutex_unlock(&mu);
}
// A queued preparation job may be promoted ahead of a demanded game job.
// The worker must re-check the game cap AFTER prioritizing that demanded job.
static void mixed_priority(void){
    VkPipeline handles[5];
    VkPipelineShaderStageCreateInfo stage={.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_VERTEX_BIT,.module=(VkShaderModule)1,.pName="main"};
    VkGraphicsPipelineCreateInfo ci={.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,.stageCount=1,.pStages=&stage};
    pthread_mutex_lock(&mu);gate=entered=finished=0;pthread_mutex_unlock(&mu);
    for(int i=0;i<4;i++){ci.subpass=i+1;assert(kva_create(device.dev,VK_NULL_HANDLE,1,&ci,&handles[i],fake_create));}
    wait_entered(3);
    replay_thread=1;ci.subpass=1001;assert(kva_create(device.dev,VK_NULL_HANDLE,1,&ci,&handles[4],fake_create));replay_thread=0;
    pthread_mutex_lock(&g_kva_mu);
    kva_pipe *normal=kva_of((uint64_t)handles[3]),*prepare=kva_of((uint64_t)handles[4]);
    assert(normal&&prepare&&g_kva_game_compiling==3);
    atomic_store(&normal->demanded,1);kva_prioritize_locked(prepare);
    pthread_cond_broadcast(&g_kva_cv);pthread_mutex_unlock(&g_kva_mu);
    usleep(80000);pthread_mutex_lock(&mu);assert(running_game==3&&entered==3);gate=1;pthread_cond_broadcast(&cv);pthread_mutex_unlock(&mu);drain();
    for(int i=0;i<5;i++)assert(kva_destroy_pipeline(device.dev,(uint64_t)handles[i],fake_destroy));
}
int main(void){
    setenv("KL_VK_ASYNC_PIPELINES","1",1);setenv("KL_VK_ASYNC_THREADS","8",1);setenv("KL_VK_ASYNC_GAME_THREADS","3",1);unsetenv("KL_VK_PIPELINE_ARCHIVE");unsetenv("KL_VK_PRIVATE_CACHE");
    device.dev=(VkDevice)&device;
    round_trip(0,16,0);round_trip(1,16,1);round_trip(0,16,1);mixed_priority();
    assert(max_game==3);assert(max_prep==8);assert(deferred_destroyed==2);
    puts("Compile budget: normal <=3, preparation 8, failure recovery, pending destroys, deferred dependencies and demanded/preparation priority race passed");return 0;
}
