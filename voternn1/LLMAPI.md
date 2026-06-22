# VoterNN LLM Access Documentation   {#llmapi}
For law and constitution generation, VoterNN requires access to a large language model.

> [!warning]
> If this format is not followed, VoterNN will not accept its results.

## Law, article generation

### Request structure
```json
{
	"prompt": "<prompt>",
	"context": "<system prompt/additional prompt>"
}
```

### Expected response
```json
{

}
```

## Law analysis
### Request structure
### Expected response

## Timeout
It is possible to set a timeout.