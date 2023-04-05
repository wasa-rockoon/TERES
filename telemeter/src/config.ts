import Vue from "vue";

declare module '@vue/runtime-core' {
    interface ComponentCustomProperties {
        $system_id: string;
    }
}

export {}
