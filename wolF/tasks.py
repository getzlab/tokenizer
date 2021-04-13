from wolf import Task

class tokenizer(Task):
    name = "tokenizer"
    inputs = { "bam", "ref_fa", "prefix" }
    script = "/app/tokenizer -b ${bam} -r ${ref_fa} -o ${prefix}.tok"
    output_patterns = { "tokens" : "*.tok" }
    docker = "gcr.io/broad-getzlab-workflows/tokenizer:v28"
    resources = { "mem" : "2GB" }
